script.lua
-- Quad position
quad_x = quad_x or 100
quad_y = quad_y or 100

function Render()
    -- Update quad position to follow mouse
    quad_x = GetMouseX() - 25
    quad_y = GetMouseY() - 25
    
    -- Draw quad
    DrawRectangle(quad_x, quad_y, 50, 50)
end

function RenderUI()
    ImGuiBegin("Controls")
    ImGuiText("Mouse X: " .. GetMouseX())
    ImGuiText("Mouse Y: " .. GetMouseY())
    ImGuiText("Quad X: " .. quad_x)
    ImGuiText("Quad Y: " .. quad_y)
    ImGuiText("Press R to reload script")
    ImGuiEnd()
end