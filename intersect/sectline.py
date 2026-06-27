import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.widgets import Slider
from matplotlib.lines import Line2D

def make_plane_normal(angle_xz, angle_y):
    """Create normal vector from two angles"""
    nx = np.sin(angle_xz) * np.cos(angle_y)
    ny = np.sin(angle_y)
    nz = np.cos(angle_xz) * np.cos(angle_y)
    n = np.array([nx, ny, nz])
    return n / np.linalg.norm(n)

def project_point(p, fov_rad, screen_w, screen_h):
    """Perspective project 3D point to 2D screen"""
    if p[2] <= 0.01:
        return None
    f = (screen_w / 2) / np.tan(fov_rad / 2)
    sx = (p[0] / p[2]) * f + screen_w / 2
    sy = -(p[1] / p[2]) * f + screen_h / 2
    return np.array([sx, sy])

def get_plane_rect_points(center_x, normal, half_w=0.5, half_h=1.5):
    """Get 4 corners of a rectangle lying on a plane centered at (center_x, 0, z)"""
    # Plane center in 3D
    # We place the plane at x=center_x, find z from normal if needed
    # Simple: center at (center_x, 0, 3)
    center = np.array([center_x, 0.0, 3.0])
    
    # Build two tangent vectors perpendicular to normal
    up = np.array([0.0, 1.0, 0.0])
    if abs(np.dot(normal, up)) > 0.99:
        up = np.array([1.0, 0.0, 0.0])
    
    right = np.cross(normal, up)
    right = right / np.linalg.norm(right)
    up2 = np.cross(right, normal)
    up2 = up2 / np.linalg.norm(up2)
    
    corners = [
        center + right * half_w + up2 * half_h,
        center - right * half_w + up2 * half_h,
        center - right * half_w - up2 * half_h,
        center + right * half_w - up2 * half_h,
    ]
    return corners

def project_polygon(corners, fov_rad, sw, sh):
    pts = []
    for c in corners:
        p = project_point(c, fov_rad, sw, sh)
        if p is None:
            return None
        pts.append(p)
    return pts

def plane_depth_coeffs(center_x, normal, fov_rad, sw, sh):
    """
    Each plane gives depth z as linear function of screen coords (sx, sy).
    z(sx, sy) = A*sx + B*sy + C
    We derive this from the plane equation: normal . (p - center) = 0
    => normal.x * x + normal.y * y + normal.z * z = normal . center
    => z = (D - nx*x - ny*y) / nz   where D = normal . center
    
    With perspective: x = (sx - sw/2) * z / f, y = -(sy - sh/2) * z / f
    Substituting:
    z = D / (nz + nx*(sx-sw/2)/f - ny*(sy-sh/2)/f)
    
    For the intersection LINE we set z_A = z_B and solve for sx, sy.
    That gives us a linear equation in sx, sy.
    
    Return (nx, ny, nz, D, f) for use in intersection calc.
    """
    center = np.array([center_x, 0.0, 3.0])
    D = np.dot(normal, center)
    f = (sw / 2) / np.tan(fov_rad / 2)
    return normal[0], normal[1], normal[2], D, f

def find_split_line(cx_a, na, cx_b, nb, fov_rad, sw, sh):
    """
    Find the 2D screen line where plane A and plane B appear to intersect.
    
    z_A(sx,sy) = D_a / (nz_a + nx_a*(sx-cx)/f - ny_a*(sy-cy)/f)
    z_B(sx,sy) = D_b / (nz_b + nx_b*(sx-cx)/f - ny_b*(sy-cy)/f)
    
    Setting equal and cross-multiplying:
    D_a * denom_B = D_b * denom_A
    
    This is linear in sx, sy -> gives us the split line equation:
    P*sx + Q*sy + R = 0
    """
    nxa, nya, nza, Da, f = plane_depth_coeffs(cx_a, na, fov_rad, sw, sh)
    nxb, nyb, nzb, Db, f = plane_depth_coeffs(cx_b, nb, fov_rad, sw, sh)
    
    cx_screen = sw / 2
    cy_screen = sh / 2
    
    # denom_A = nza + nxa*(sx-cx_screen)/f - nya*(sy-cy_screen)/f
    # denom_B = nzb + nxb*(sx-cx_screen)/f - nyb*(sy-cy_screen)/f
    # Da * denom_B = Db * denom_A
    # Da*(nzb + nxb*(sx-cx_screen)/f - nyb*(sy-cy_screen)/f) = 
    # Db*(nza + nxa*(sx-cx_screen)/f - nya*(sy-cy_screen)/f)
    
    # Collect sx terms: (Da*nxb - Db*nxa)/f
    # Collect sy terms: (-Da*nyb + Db*nya)/f
    # Constant: Da*nzb - Db*nza + (Da*nxb - Db*nxa)*(-cx_screen)/f + (-Da*nyb+Db*nya)*(-cy_screen)/f... 
    # Let's just expand cleanly:
    
    P = (Da * nxb - Db * nxa) / f
    Q = (-Da * nyb + Db * nya) / f
    R = Da * nzb - Db * nza + (Da * nxb - Db * nxa) * (-cx_screen) / f + (-Da * nyb + Db * nya) * (-cy_screen) / f
    # Wait, let me redo: constant terms from expanding denom_A and denom_B
    # denom_A = nza + nxa/f * sx - nxa/f*cx_screen - nya/f*sy + nya/f*cy_screen
    # = nza - nxa*cx_screen/f + nya*cy_screen/f  +  nxa/f * sx  - nya/f * sy
    # Let a0 = nza - nxa*cx_screen/f + nya*cy_screen/f
    # Let ax = nxa/f,  ay = -nya/f
    # denom_A = a0 + ax*sx + ay*sy
    # similarly for B
    
    a0 = nza - nxa * cx_screen / f + nya * cy_screen / f
    ax = nxa / f
    ay = -nya / f
    
    b0 = nzb - nxb * cx_screen / f + nyb * cy_screen / f
    bx = nxb / f
    by_ = -nyb / f
    
    # Da*(b0 + bx*sx + by_*sy) = Db*(a0 + ax*sx + ay*sy)
    # (Da*bx - Db*ax)*sx + (Da*by_ - Db*ay)*sy + (Da*b0 - Db*a0) = 0
    
    P = Da * bx - Db * ax
    Q = Da * by_ - Db * ay
    R = Da * b0 - Db * a0
    
    return P, Q, R

def line_from_implicit(P, Q, R, sw, sh, margin=200):
    """Convert Psx + Qsy + R = 0 to two screen points for drawing"""
    # If |P| > |Q|, parameterize by sy, else by sx
    pts = []
    sy_vals = [-margin, sh + margin]
    sx_vals = [-margin, sw + margin]
    
    if abs(Q) > abs(P):
        # sx = -(Q*sy + R) / P
        for sy in sy_vals:
            if abs(P) < 1e-10:
                continue
            sx = -(Q * sy + R) / P
            pts.append((sx, sy))
    else:
        # sy = -(P*sx + R) / Q
        for sx in sx_vals:
            if abs(Q) < 1e-10:
                continue
            sy = -(P * sx + R) / Q
            pts.append((sx, sy))
    
    if len(pts) < 2:
        # Degenerate, try other axis
        for sx in sx_vals:
            if abs(Q) < 1e-10:
                break
            sy = -(P * sx + R) / Q
            pts.append((sx, sy))
    
    return pts

# ---- Setup ----
SW, SH = 800, 600
FOV = np.radians(120)

fig, ax = plt.subplots(figsize=(10, 8))
plt.subplots_adjust(bottom=0.3)
ax.set_xlim(0, SW)
ax.set_ylim(SH, 0)
ax.set_aspect('equal')
ax.set_facecolor('#111111')

poly_a_patch = plt.Polygon([[0,0]], closed=True, fill=True, 
                             facecolor=(1,0.2,0.2,0.3), edgecolor='red', linewidth=2)
poly_b_patch = plt.Polygon([[0,0]], closed=True, fill=True,
                             facecolor=(0.2,0.4,1,0.3), edgecolor='blue', linewidth=2)
split_line = Line2D([0,1],[0,1], color='yellow', linewidth=2, linestyle='--')

ax.add_patch(poly_a_patch)
ax.add_patch(poly_b_patch)
ax.add_line(split_line)
title = ax.set_title('', color='white')
fig.patch.set_facecolor('#222222')
ax.tick_params(colors='white')

# Sliders
ax_s1 = plt.axes([0.15, 0.20, 0.7, 0.03])
ax_s2 = plt.axes([0.15, 0.15, 0.7, 0.03])
ax_s3 = plt.axes([0.15, 0.10, 0.7, 0.03])
ax_s4 = plt.axes([0.15, 0.05, 0.7, 0.03])

s_axz_a = Slider(ax_s1, 'Plane A xz angle', -80, 80, valinit=20, color='#aa3333')
s_ay_a  = Slider(ax_s2, 'Plane A y angle',  -60, 60, valinit=0,  color='#aa3333')
s_axz_b = Slider(ax_s3, 'Plane B xz angle', -80, 80, valinit=-20, color='#3344aa')
s_ay_b  = Slider(ax_s4, 'Plane B y angle',  -60, 60, valinit=0,  color='#3344aa')

for s in [s_axz_a, s_ay_a, s_axz_b, s_ay_b]:
    s.label.set_color('white')
    s.valtext.set_color('white')

def update(_):
    na = make_plane_normal(np.radians(s_axz_a.val), np.radians(s_ay_a.val))
    nb = make_plane_normal(np.radians(s_axz_b.val), np.radians(s_ay_b.val))
    
    corners_a = get_plane_rect_points(-1.0, na)
    corners_b = get_plane_rect_points( 1.0, nb)
    
    pts_a = project_polygon(corners_a, FOV, SW, SH)
    pts_b = project_polygon(corners_b, FOV, SW, SH)
    
    if pts_a:
        poly_a_patch.set_xy(pts_a)
    if pts_b:
        poly_b_patch.set_xy(pts_b)
    
    P, Q, R = find_split_line(-1.0, na, 1.0, nb, FOV, SW, SH)
    
    line_pts = line_from_implicit(P, Q, R, SW, SH)
    if len(line_pts) >= 2:
        xs = [line_pts[0][0], line_pts[1][0]]
        ys = [line_pts[0][1], line_pts[1][1]]
        split_line.set_data(xs, ys)
    
    mag = np.sqrt(P**2 + Q**2)
    title.set_text(f'Yellow = apparent intersection line   |   Line eq: {P:.2f}·x + {Q:.2f}·y + {R:.2f} = 0')
    
    fig.canvas.draw_idle()

s_axz_a.on_changed(update)
s_ay_a.on_changed(update)
s_axz_b.on_changed(update)
s_ay_b.on_changed(update)

update(None)

ax.legend(handles=[
    patches.Patch(facecolor='red', alpha=0.5, label='Plane A (x=-1)'),
    patches.Patch(facecolor='blue', alpha=0.5, label='Plane B (x=+1)'),
    Line2D([0],[0], color='yellow', linestyle='--', label='Split line'),
], facecolor='#333333', labelcolor='white')

plt.show()
