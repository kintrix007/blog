---
post-title: Setting Up Flutter for Android Development on NixOS
---

# Setting Up Flutter Android on NixOS

Recently I, together with friends, set out to make a cross-platform mobile app
using Flutter. For that of course I needed to do a couple things first:

1. Set up Flutter and its dependencies (incl. Android SDK)

2. Learn Flutter (this we will skip)

## Setting up Flutter

This is actually something I have done this before back in March, 2024. I had
the idea to learn Flutter back then, because I wanted a practical reason to
learn Dart[^1].

[^1]: Flutter uses Dart as the programming language.

That time, it was a really short-lived dream, as I ended up giving up on it not
much after just figuring out how to set it up. I recall, all I did was build the
example project as a web and Android app, then tweaked some of the text on the
main page. But that is useful, as I can reference back to it to figure out what
I did.

Installing Flutter itself is very simple. Just add the `flutter` package to a
Nix shell, and you are good to go. Either run `nix-shell -p flutter` to get a
shell temporarily, or create a `shell.nix` file and then you can run `nix-shell`
by itself:

```nix
# shell.nix

with import <nixpkgs> {};

mkShell {
  packages = [
    flutter
  ];
}
```

Easy as pie.

### Creating a Project

TODO

## Getting the Android SDK

Now we just need to get the Android SDK, and we can build our example project
with Flutter.

### First Futile Attempt

Here is where I started to make a big mistake: assume I can just get the Android
SDK using Nix. Surely, I can just add it to the packages list, I thought.

After a short bit of searching, I found that there is indeed a package called
`androidsdk` in the nixpkgs repo. Sounds perfect. I edited my `shell.nix`

```nix
# shell.nix

with import <nixpkgs> {};

mkShell {
  packages = [
    flutter
    androidsdk
  ];
}
```

And then ran `nix-shell`, clearly oblivious. Immediately got my terminal
flooded. It vomited out the entire Android SDK license onto `stdout` followed by this:

```log
       You must accept the following licenses:
         - android-sdk-license

       a)
         by setting nixpkgs config option 'android_sdk.accept_license = true;'.
       b)
         by an environment variable for a single invocation of the nix tools.
           $ export NIXPKGS_ACCEPT_ANDROID_SDK_LICENSE=1
```

I do not like the environment variable option, I would rather accept the terms
using Nix either way. So I will go with option a). While it does not tell you
where to put that snippet of code, having been using Nix for a while I knew what
I could do with it.

My immediate thought was that I can just put it in
`~/.config/nixpkgs/config.nix`, which would work. One could argue that it is the
correct solution, since accepting the terms should not be part of the nix shell
automatically, but it is something each user should do themselves. But let's try
being naughty and still make it part of the shell so that we never have to deal
with it again.

```nix
# shell.nix

with import <nixpkgs> {
  config.android_sdk.accept_license = true;
  config.allowUnfree = true;
};

mkShell {
  packages = [
    flutter
    androidsdk
  ];
}
```

Note that I also set `allowUnfree` to `true`, since `androidsdk` is a non-free
package.

After a quick little `nix-shell`, it seems to work. But it is taking a while...

```log
[...]
copying path '/nix/store/s3yjazzkqjh17f3i4ahcn1963s0s8srd-arm64-v8a-32_r08.zip' from 'https://cache.nixos.org'...
copying path '/nix/store/wpvkqjvrcm7xlsnhv2661qi2zxy0iq9y-arm64-v8a-33_r09.zip' from 'https://cache.nixos.org'...
copying path '/nix/store/kr1rhlrjwgi2376p9icfr551wyxpcx47-arm64-v8a-33_r17.zip' from 'https://cache.nixos.org'...
copying path '/nix/store/7xqwbr2r9bvcy1vl5jyqvi7fqf1cgv8v-arm64-v8a-34_r14.zip' from 'https://cache.nixos.org'...
copying path '/nix/store/fgm7fqhd0q94bzj508kgjqp6xvam85hk-arm64-v8a-34_r14.zip' from 'https://cache.nixos.org'...
copying path '/nix/store/fq8blynrm6xcf71g3x7j1gv7ka80c89f-arm64-v8a-35_r09.zip' from 'https://cache.nixos.org'...
copying path '/nix/store/ys9hc9r69js68gprci2ssq2v4sfykh3k-arm64-v8a-35_r09.zip' from 'https://cache.nixos.org'...
copying path '/nix/store/fa7myxvrvc9hhjbccvd3nx023xqdal1g-arm64-v8a-36_r06.zip' from 'https://cache.nixos.org'...
copying path '/nix/store/yqfppq2cv6wq8mz7afdlbhmzw3kvfb7h-arm64-v8a-36_r06.zip' from 'https://cache.nixos.org'...
[...]
```

Huh... That is a lot of things getting downloaded. After waiting for a couple
minutes, the nix-shell process crashed with "not enough disk space". Great. That
means it was trying to download at least 80GB worth of "android SDK". For some
reason, nixpkgs is on the opinion that the `androidsdk` package should already
contain everything that you may have possibly wanted, even it if takes and
outrageous amount of space.

### Making Our Own `androidsdk` Package

Okay, let's try to work around that. After checking how `androidsdk` is defined,
we find that it is in fact `androidenv.androidPkgs.androidsdk`. That's good to
know. We can find that `androidend.androidPkgs` is defined [like
this](https://github.com/NixOS/nixpkgs/blob/29e290002bfff26af1db6f64d070698019460302/pkgs/development/mobile/androidenv/default.nix#L20)
in nixpkgs.

Aha! This explains things.

```nix
composeAndroidPackages {
  # Support roughly the last 5 years of Android packages and system images by default in nixpkgs.
  numLatestPlatformVersions = 5;
  includeEmulator = "if-supported";
  includeSystemImages = "if-supported";
  includeNDK = "if-supported";
};
```

It seems the maintainers are very well aware of what they are doing. They are
purposely making it contain "roughly the last 5 years of Android packages and
system images".

Well, now that we know how it is defined, we can just use this in our Nix shell
instead. Let's rework it.

```nix
# shell.nix

with import <nixpkgs> {
  config.android_sdk.accept_license = true;
  config.allowUnfree = true;
};

let
  androidPkgs = androidenv.composeAndroidPackages {
    numLatestPlatformVersions = 1;
    includeEmulator = true;
    includeSystemImages = true;
    includeNDK = false;
  };
in
mkShell {
  packages = [
    flutter
    # This the 'androidPkgs' defined above
    androidPkgs.androidsdk
  ];
}
```

After encouraging it with a little `nix-shell` and less than a minute of
waiting, it is done, and put me in a new shell.

Let's verify that it has worked, just to be sure. We can try invoking an
executable from the SDK.

First, let's find out what binaries it contains... Or rather where the android
SDK we just added is. It will be referenced by our shell, so we can just extract
it.

```sh
$ nix-store -q --references $(nix-build shell.nix) | grep android
/nix/store/6ixvbl3p75xsmxl3h9977xb3c3lmfhba-androidsdk
```

There it is! Let's see what executables it has:

```sh
$ ls /nix/store/6ixvbl3p75xsmxl3h9977xb3c3lmfhba-androidsdk
bin  libexec
$ ls /nix/store/6ixvbl3p75xsmxl3h9977xb3c3lmfhba-androidsdk/bin
adb          d8        hprof-conv  make_f2fs_casefold  r8                screenshot2
apkanalyzer  etc1tool  lint        mke2fs              resourceshrinker  sdkmanager
avdmanager   fastboot  make_f2fs   profgen             retrace
```

Alright, so we can try calling `adb --version` to verify that it got correctly
added to our environment. And it did! Awesome.

### Try to build with it

---

TODO

---

Alright, so we need to set the `ANDROID_HOME` variable to point to the SDK. That
is actually very simple to do.

```nix
# shell.nix

# [...]
mkShell {
  packages = [
    flutter
    androidPkgs.androidsdk
  ];

  ANDROID_HOME = "${androidPkgs.androidsdk}";
}
```

And voila, after entering the shell with `nix-shell`, we can see that the
environment variable is set.

```sh
$ echo "$ANDROID_HOME" 
/nix/store/6ixvbl3p75xsmxl3h9977xb3c3lmfhba-androidsdk
```

### Accepting the License

Impossible.

### Give up

```nix
# shell.nix

with import <nixpkgs> { };

mkShell {
  buildInputs = [
    flutter
    sdkmanager
  ];

  ANDROID_HOME = "${builtins.getEnv "HOME"}/.androidsdk";
}
```

And then use `sdkmanager` to install things to `$ANDROID_HOME`.

```sh
sdkmanager --install 'platform-tools' # or whatever else
```

---
