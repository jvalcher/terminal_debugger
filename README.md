
# termvu

A multi-language TUI debugger

**\*\* In development \*\***


## Configuration

Unique, three-character, case-sensitive "plugin" codes are each associated with a specific debugger action or window. The codes are bound to user-defined shortcut keys. The keys are then used to create custom ASCII-art layouts.
<br />

### Example configuration

```
[ plugins ]

Asm : a : (a)ssembly
Brk : e : br(e)akpoints
Con : c : (c)ontinue
Dbg : d : (d)ebug out
Fin : f : (f)inish
Kil : k : (k)ill
Lay : l : (l)ayouts
LcV : v : local (v)ars
Nxt : n : (n)ext
Prg : p : (p)rogram out
Qut : q : (q)uit
Reg : g : re(g)isters
Run : r : (r)un
Src : o : s(o)urce file
Stp : s : (s)tep
Wat : w : (w)atch

[ layout : Main ]

>h
lq
rnscfk
>w
eoo
woo
vpd

[ layout : Assembly / Registers ]

>h
lq
rnscfk
>w
oag

```


### Resulting layouts

<img src='./misc/layout1.png' height='500px'>
<img src='./misc/layout2.png' height='500px'>
<br />


### Configuration file location

`$PROJECT_DIR/.termvu` <br />
or <br />
`$HOME/.termvu`
<br /><br />
 

## Build

### Production
```
$ make
```
### Development
```
$ make dev
```
<br /><br />

