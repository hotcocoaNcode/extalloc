# extalloc
>*ooga booga shit fuck production edition*

Have you ever spent a week making a library for something you THOUGHT didn't already exist 
but then at the very end of development while typing the name of your own library 
you see what you were looking for in the standard?

No?
Not really?

Well I did that. And based on all tests and sanity checks I have run,
```
(our) init took 32 cycles
theirs took 6874 cycles
sanity check; 0x7000000C throwaway 0x700000A8 real
ours took 65 cycles
sanity check; 0x70000000 throwaway 0x70000080 real
```
We are significantly faster than the standard Teensy 4.1 `extmem_malloc`.

I don't know why. It doesn't feel like it should be. I'm putting this on github so maybe someday,
someone can tell me what was broken with my testing and why the results favor me.

For now I will be proud of this. For now.