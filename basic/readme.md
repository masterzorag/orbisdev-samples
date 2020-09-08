### basic sample

This is meant to test if your setup (set of patches/hooks you are applying) support the following:
- mount /data path
- fself spawn
- loading and patching of piglet modules from /data/self path
0. Read https://github.com/orbisdev/orbisdev-orbislink/blob/master/README.md for OrbisLink and debugNet reference.

_If Retroarch_r4 works, you will need just the orbislink-config.ini in place_

1. Install basic fpkg;
2. Execute app: you should get one single beep (if not, wrong setup)

_If you get an additional triple beep restart from point 0._

3. Listen debugnet on pc;

4. App enters a black screen loop: pad buttons on debugNet, Triangle button exit app.



