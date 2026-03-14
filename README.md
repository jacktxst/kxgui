# kxgui
cross platform, any graphical backend, immediate mode gui c header library

[click on this link to see a live demo of kxgui running in your browser thanks to WebAssembly](https://jacktxst.github.com/kxgui/wasm-demo)

kxgui is a C library for adding GUIs to any application on any platform.
kxgui itself relies on no libraries or graphics api. to actually get the
frame that kxgui produces to show up on the screen, a renderer function
is required in order to translate kxgui's output into the appropriate
system-specific memory management and draw calls.


kxgui is an immediate mode gui library inspired by libraries like [clay](https://github.com/nicbarker/clay) and [dear imgui](https://github.com/ocornut/imgui).
when using an immediate mode library, you do NOT create a tree-like data structure and populate it with nodes representing GUI elements, as you would in a retain mode gui system.
instead, the program's main loop calls functions for drawing GUI components on every frame.

```
.. in your main rendering loop ..
kxgui_begin_frame()

if (kxgui_button("quit game")) {
  quit_game()
}
kxgui_label("hello world")
...
```

the two essential elements of any GUI built using kxgui are
components and containers. containers are also components
but their behavior varies from all other components in that
they can "contain" other components and in turn, containers.

```
.. in your main rendering loop ..
kxgui_begin_container()


	kxgui_label("inside the container")


	kxgui_begin_container()

		kxgui_label("inside a container inside a container!")

	kxgui_end_container()


kxgui_end_container()


kxgui_label("outside the container")
...
```

please read kxgui.h for more info



