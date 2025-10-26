local rects = {}
if not animTime then animTime = 0 end

-- Restore persistent rects on reload
local persistentRects = GetPersistentRects()
for i, rectIndex in ipairs(persistentRects) do
    table.insert(rects, rectIndex)
end

function Render()
    animTime = animTime + 0.016
    local x = 100 + math.sin(animTime) * 50
    DrawRectangle(x, 100, 200, 150)

    if GetKeyPressed and GetKeyPressed(32) then -- Space key
        for i = 1, 10000 do
            local x = math.random(0, 700)
            local y = math.random(0, 500)
            local w = math.random(120, 480)
            local h = math.random(120, 480)
            local r = math.random(0, 255)
            local g = math.random(0, 255)
            local b = math.random(0, 255)
            local a = math.random(5, 15)

            local index = SpawnTransparentRect(x, y, w, h, r, g, b, a)
            table.insert(rects, index)
            AddPersistentRect(index) -- Store in C++
        end
    end

    -- Move rectangles
    for i, rectIndex in ipairs(rects) do
        local time = GetTime and GetTime() or 0
        local x = 400 + math.sin(time + i * 0.1) * 1200
        local y = 300 + math.cos(time + i * 0.15) * 1150
        SetRectPosition(rectIndex, x, y)
    end

    if GetKeyPressed and GetKeyPressed(67) then -- C key
        ClearAllRects()
        ClearPersistentRects()
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
