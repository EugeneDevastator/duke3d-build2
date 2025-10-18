function Render()
    DrawRectangle(100, 100, 200, 150)
end

function RenderUI()
    ImGuiBegin("Test Window")
    ImGuiText("Hello from Lua!")
    ImGuiText("Mouse: " .. GetMouseX() .. ", " .. GetMouseY())
    ImGuiEnd()
end