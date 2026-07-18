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

### Formatting

- Formatting is absolutely trash
- I have to configure it so it looks like the much more readable C#

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

### Centralizing text display

- Having a single function to display text creates time of clear issue with the text buffer
	- clear the text buffer at the start of the update method that's going to run

### Audio

- Audio effects can be added as .wav
- Background audio only works with some complicated file format that I couldn't find

### trigonometry

- for some reason, the default trigonometry operations in butano don't get you degrees
- use `bn::degrees_` instead of the normal versions to get a usable result

### Speculative collision and collider passthrough

- very fast objects can go through colliders without intersecting them
- 2 ways to fix that
	- interpolate trajectory with x steps (have to configure this, physics solver steps ?) and check if at least one step has collision
	- compute intersection point and check if it has collision (a lot of costly calculus)

### Sprites end of life

- Not sure how I'm supposed to manage that
	- `bn::optional<>.reset()` doesn't seem to work (I get an "invalid optional" error from the emulator)

### bn::fixed imprecisions

- `bn::fixed` is imprecise as hell, resulting in percentile calculations exceeding 1 and creating errors
	- should I make a custom function to make sure percentiles are always in range

### Documentation

- Butano's documentation is severely lacking, especially when it comes to basic language support
	- `BN_LOG()` doesn't support basic types such as `float` and `double`
- A lot of functionalities are not documented properly, especially on how to use them / what they do
- What the fuck is an affine or a H-blank ?
	- am I missing basic knowledge here ?


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

### Custom log tool

Custom log tool similar to what I made in ArtOfRally_ModBase
- Log at interesting places for control flow
- Enable / disable logging easily
- Tag logs for better readability

### Particle system

System to animate particles in a very simple way
- needs to make it very very light
- needs to be very lightweight in terms of memory
- gravity and animations