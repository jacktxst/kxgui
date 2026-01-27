# kxgui
cross platform, any graphical backend, immediate mode gui c header library

kxgui is an immediate mode gui library inspired by libraries like [clay](https://github.com/nicbarker/clay) and [dear imgui](https://github.com/ocornut/imgui).
when using an immediate mode library, your loop needs to call functions for drawing components every frame.

```
.. in your main rendering loop ..
kxgui_begin_frame()

if (kxgui_button("quit game")) {
  quit_game()
}
kxgui_label("hello world")
...
```

this kind of pattern makes for very fast iteration times for development.
it's also extremely easy to create ui that responds to the programs state,
because gui commands can be skipped by if-else conditional branching or
duplicated by any kind of loop. you can also write functions that contain
calls to kxgui component functions to build interfaces. you can also create
your own kxgui components using a special api.

# rendering / wrapping
kxgui by itself is missing a few of the things required to be usable. 
you will need to either find or create a renderer as well as some kind of user input wrapping.

i'm currently working on a renderer and input wrapping for sokol which i will include in this repo.

# using

once you have the renderer and the input wrapping, you just need a struct kxgui_context
and to call kxgui_begin_frame()

then you just issue components by calling their associated functions.
some stateful components require you to maintain a static struct in your own memory scope.
this is because kxgui strives not to ever allocate or perform significant memory management under the hood.

kxgui will lay out all of your components, respecting the size of the screen as well as each component and container.
it allows for containers within containers within containers within container (and so on) and all of these can be scrollable.

then use your renderer to render the output of kxgui

# how?

i will work on some slight documentation but the header files are supposed to be self documenting

