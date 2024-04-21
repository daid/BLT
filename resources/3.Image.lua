function sleep(x) for n=1,x do yield() end end

setFallSpeed(0.1)
spawnImage("3.Image.png")


showMessage("3.Image", true)
sleep(300)
showMessage("")

while getMinY() < 20 do
    yield()
end

sleep(120)

victory()
