# Using linters with Neovim

<!--toc:start-->
- [Using linters with Neovim](#using-linters-with-neovim)
  - [My History with Vim](#my-history-with-vim)
  - [Why Configure the Linter Support](#why-configure-the-linter-support)
  - [How to Configure the Linter Support](#how-to-configure-the-linter-support)
    - [Installing nvim-lint](#installing-nvim-lint)
    - [Configuring nvim-lint](#configuring-nvim-lint)
<!--toc:end-->

I really like using Vim as my text editor. I am saying *text* editor instead
of *code* editor because I use it for not just coding. For example, I am
writing this blog right now using Neovim. It is a wonderful editor that really
grew on me.

## My History with Vim

I have first heard of Vim (you know, the one that starts with `VIM - Vi
IMproved`) and found out what it is when I first started dual-booting Ubuntu in
2020. However, I only started using instead of something like [VS
Code](https://code.visualstudio.com/) in 2022 during my freshman year of
university when I needed to write some x86 assembly. It is not like a fancy IDE
would have given me better suggestions, so I decided to write all my
assignments in plain-old Vim. And quite frankly, I didn't have a bad time with
it.

I have found [this](https://github.com/HealsCodes/vim-gas) syntax highlighting
plugin for GNU `as`, and I installing it by `git clone`-ing it in the correct
location. And it just worked as it should. At this point my understanding of Vim
was that I can toggle between modes and edit like that. At this point the
commands I was familiar with were basically limited to: `i`, `a`, `v`, `y`, `p`
and `d`. So, just enough to get you going.

Later, after I ~~up~~downgraded to [a very low-power
laptop](https://pine64.org/devices/pinebook_pro/) I wanted to have a decent
experience editing code. So I set out to learn Vim. After a short bit of
research, I realized that I am probably better of using
[Neovim](https://neovim.io/), which is a more modern version of Vim that also
allows you to write your config in [Lua](https://www.lua.org/) instead of
Vimscript.

Turns out, you can do a lot with Neovim. I mean a lot.

## Why Configure the Linter Support

One of the first things I did was installing treesitter and make Neovim work as
a LSP (Language Server Protocol) client for the languages I actually use. At
the time I was using [lsp-zero](https://github.com/VonHeikemen/lsp-zero.nvim),
but currently I am just directly using Neovim's built-in LSP support. Which is
surprisingly simple.

That is a great start, with that I already had nice syntax highlighting thanks
to treesitter, and all of the LSP goodies like autocompletion, auto-renaming
variables, static error messages showing up right next to the relevant line and
so on. And many LSPs support interfacing with some linters too, so this meant
linter support for many of the programming languages too.

But not all. And more importantly, this way I could only have support for
linters if there is an LSP for it. What about linting my markup languages like
JSON and YAML? What about linting my Markdown? This last one is particularly
important to me because I spend a lot of time writing Markdown. I write my blog
posts in Markdown too!

Well of course I could always just exit Neovim, then run `markdownlint file.md`
like some sort of an caveman who had to carve their Markdown in the walls... But
I want to have a better experience than that. I want the linter messages to show
up as diagnostics in my editor just like with LSPs. For example, I want it to
highlight a line that has wrong formatting or an accidental trailing space.

## How to Configure the Linter Support

As far as I am aware, Neovim does not have an official API for communicating
with linters. Luckily, however, there is
[nvim-lint](https://github.com/mfussenegger/nvim-lint), a plugin to turn linter
output into Neovim diagnostics. There are only 2 things we need to do:

1. Install it with our preferred plugin manager

2. Configure it to run linters for set file-types

### Installing nvim-lint

TODO: Write

### Configuring nvim-lint

TODO: Write
