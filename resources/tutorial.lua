function sleep(x) for n=1,x do yield() end end

setOverlayAlpha(1.0)
sleep(120)

showMessage("Welcome\nto BLT")

for n=1,300 do
    yield()
    setOverlayAlpha(1-n/300)
end
sleep(120)

showMessage(
[[Here we will teach
 you all about
writting binary]])
sleep(300)

clearInput()
showMessage(
[[First, let us
talk about zero.
To create a zero
press the zero key

Which is [0] or
 [left shift] ]])

while not string.find(getInput(), "0") do
    if string.find(getInput(), "1") then
        clearInput()
        showMessage(
[[No, that is a one

Press [0] or
 [left shift] ]])
    end
    yield()
end

showMessage(
[[Good, now
fire that zero
by pressing
 [space] ]])
while string.len(getInput()) > 0 do
    yield()
end
showMessage([[And off it goes!]])
sleep(120)

clearInput()
showMessage(
[[Next, let us write
a one. Just press:
[1] or [right shift] ]])

while not string.find(getInput(), "1") do
    if string.find(getInput(), "0") then
        clearInput()
        showMessage(
[[No, that is a zero.

Press [1] or
 [right shift] ]])
    end
    yield()
end
showMessage(
[[Great, now send
that off as well]])
while string.len(getInput()) > 0 do
    yield()
end
showMessage(
[[Now, enter 8 ones
or zeros without
sending them]])
while string.len(getInput()) < 8 do
    yield()
end
showMessage(
[[See how you can
combine multiple
ones and zeros
into a bigger
total]])
sleep(300)

setFallSpeed(0.0)
showMessage(
[[How, I have
a single zero
for you here

Fire a zero at!]])
spawn({0, 0}, 0)
while getMinY() < 20 do
    yield()
end
showMessage(
[[   !KAPOOW!
There it goes]])
sleep(120)
showMessage(
[[Good.
Your goal is
to destroy all
the ones and zeros
before they reach
the bottom of
the screen]])
sleep(300)

showMessage(
[[Here are some
practice binaries]])
setFallSpeed(0.05)
for y=-2,2 do
    for x=-2,2 do
        spawn({x, y}, irandom(0, 1))
    end
end
while getMinY() < 20 do
    yield()
end

showMessage(
[[Well done!
You are now ready
for the real work!

(End of demo)]])
sleep(120)

showMessage("")
for y=-2,42 do
    for x=-3,3 do
        spawn({x, y}, irandom(0, 1))
    end
end
setFallSpeed(0.3)
