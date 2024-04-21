function sleep(x) for n=1,x do yield() end end

setFallSpeed(0.4)
for y=0,15 do
    for x=-3,4 do
        spawn({x-.5, y+10}, irandom(0, 1))
    end
end


showMessage("4.Endless", true)
sleep(300)
showMessage("")

while true do
    yield()
    if getMaxY() < 20 then
        local y = getMaxY();
        print(y)
        for x=-3,4 do
            spawn({x-.5, y+1}, irandom(0, 1))
        end
    end
end

sleep(120)

victory()
