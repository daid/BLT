function sleep(x) for n=1,x do yield() end end

setFallSpeed(0.4)
for y=0,30 do
    for x=-2,3 do
        spawn({x-.5, y+10}, irandom(0, 1))
    end
end


showMessage("2.Medium", true)
sleep(300)
showMessage("")

while getMinY() < 20 do
    yield()
end

sleep(120)

victory()
