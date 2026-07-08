Butano Pong
===

I'm remaking Pong for the GBA using Butano.\
This is a learning project, things will be messy.


## Target

- Full pong gameplay
- Intro text
- Sounds
- Results screen
- Restart


## Best practices

### Screen placement

- `Canvas` class to position objects on screen is directly copied from **UGUI** systems (except I keep the center reference point)
	- feed it screen position percent as float
	- split into point (x or y) and position (x and y)
- Replace `float` usage with butano's `bn::fixed` (from **butano docs** "poor man's float")

### Use Butano's built-in classes

- Avoid using pointers when Butano already sends you `bn::sprite_ptr` (wrapper around smart pointers)

### Headers

- Make headers for all of your `.cpp` files
	- You won't have to care about functions order


## Tribulations

### Includes

- Always explicitely have all includes used in this file (no implicit includes through other header files)

### Declarations

- Declaring `bn::sprite_ptr` without initializing them doesn't work, but Butano has optionals built in
	- Declare `bn::optional<>` instead and assign it with `bn::sprite_ptr.generate_sprite_optional()`

### Procedural animations

- Use `bn::timer` class to count ticks
	- use `bn::timers::ticks_per_frame()` to convert ticks to frames
	- use `bn::timers::ticks_per_second()` to convert ticks to seconds
	- don't forget to cast one of those to `bn::fixed` to get something usefull
- You can't % by a non-integer number (`bn::fixed` for example), but you can divide the timer by your desired % value


## Tools ideas

### Akashic

Manages various assets references based on user defined game state.
- User can define enum (int) to define game state and cast it to int as a way to identify assets
- All references are cleared properly when changing game state

### Display manager

Helps users to place sprites on the screen.
- User can define an offset or a new center to a sprite
- Place objects on screen based on screen percents

### Animator

Some kind of tool so I can describe animations and apply those animations to some designated properties.
- User can describe procedural animations per second
- Animate any property proceduraly
	- set start & end values
	- custom procedure defined by user