local rects = {}

function Render()
    -- Spawn test rectangles on space key
    if GetKeyPressed and GetKeyPressed(32) then -- Space key
        for i = 1, 100 do
            local x = math.random(0, 700)
            local y = math.random(0, 500)
            local w = math.random(20, 80)
            local h = math.random(20, 80)
            local r = math.random(0, 255)
            local g = math.random(0, 255)
            local b = math.random(0, 255)
            local a = math.random(50, 150) -- Semi-transparent
            
            local index = SpawnTransparentRect(x, y, w, h, r, g, b, a)
            table.insert(rects, index)
        end
    end
    
    -- Move rectangles
    for i, rectIndex in ipairs(rects) do
        local time = GetTime and GetTime() or 0
        local x = 400 + math.sin(time + i * 0.1) * 200
        local y = 300 + math.cos(time + i * 0.15) * 150
        SetRectPosition(rectIndex, x, y)
    end
    
    -- Clear on C key
    if GetKeyPressed and GetKeyPressed(67) then -- C key
        ClearAllRects()
        rects = {}
    end
end

function RenderUI()
    ImGuiBegin("Benchmark Control")
    ImGuiText("Press SPACE to spawn 100 rects")
    ImGuiText("Press C to clear all rects")
    ImGuiText("Press R to reload script")
    ImGuiText("Rect Count: " .. GetRectCount())
    ImGuiText("Mouse: " .. GetMouseX() .. ", " .. GetMouseY())
    ImGuiEnd()
end
