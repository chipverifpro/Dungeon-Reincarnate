## Dungeon Reincarnate

### Background:

When I was in middle school, I wrote a program called Dungeon on my Commodore 64.  It was partially
programmed in Basic (the waiting for keyboard commands and the printing of text), but the majority
was in 6502 Assembly Language because the machine needed the speed to draw the graphics and do the
route tracking, etc. in real-time.  It took several years and I loved every minute of it.
Some of the features:
* Main window had an overhead map view.
* Only the visible area was shown, everything else was black.
* A group of 3 characters were drawn as top-down stick figures.
* Player control consisted of keys to indicate move direction, or other actions.
* Monsters had a route-planning algorithm and they would follow it to the players.
  * Monsters were smart.  They wandered aimlessly unless they knew where the players were.
  * I really thought I had come up with a great new algorithm for the route tracking, but I found
    out years later, this was a common route-planning algorithm that I re-invented.
  * I loved playing with the route following, and would watch the monsters navigate complex mazes
    direct to their destination. 
  * Some monsters were dumb and couldn't open doors.  Others were smart and could.
* A secondary window showed messages about what the players were doing or seeing.
* In battles, the view shifted to an enlarged map.
  * Every nearby tile was auto-expanded to a 3x3 set of tiles, then the game worked
    as it had in the main map, same graphics tiles and everything.
  * The 3 characters split into separate figures and everyone took turns moving and attacking
* Data structures were very compact and used every bit available.  Memory was a precious commodity.
   * Maps were one byte for each cell: 4 bits for walls in each direction and 4 bits for doors.
   * Another array had one byte for items and codes for special things about the cell.
   * 2 additional arrays the same size as the map were used for everything else dynamically.
     * Route planning got a byte for distance to target at each cell
     * Maps were decoded to graphics for each cell as well.
     * Other things used these temporary spaces, but I don't remember exactly what those were.

### 2024-2025:

I searched long and hard for my notes, maps, sourcecode, everything but was unable to find even a trace.
I decided I needed to re-create this with a more modern touch, but preserve the look and feel of the
original.  I had that look and feel, algorithms, everything burned in my memory so it wasn't hard to
pull out.

I chose C as the programming language because it had the low-level capabilities to match with
the good old assembly I had originally used.  I could follow similar code structures, data arrays, etc.
I chose SDL2 as a graphics library for portability, planning to run it on my cellphone eventually.
I kept the data structures mostly intact, but I wanted to add to the graphics things like fancy walls,
objects stacked in cells, keepout areas like not allowing players to walk through a fountain, etc.
The data format is a struct that is stored for each cell like the old 1-byte map.  The view got color,
custom wall textures, more flexible objects, things move a small step at a time instead of a whole
tile, and more monsters with varying capabilities.  The whole view could zoom in or out as desired,
great for seeing the explored areas of the map, or detailed look at an object in the room.  Players
select where to go by using a mouse, and letting the route tracking algorithm do the work of getting
there.

I didn't want to do the same battle mechanic as
before, because that felt hard to do on a cellphone screen.  I came up with a simple set of 2
target circles that had dots bouncing around them.  On the monster's circle, each one represents a
monster.  In the player's circle, each represents a character.  The player taps when they think most
of the monster's dots are inside the target.  The inner circle is a critical hit, the outer circle
is a regular hit, and beyond that is a miss.  The monsters would each randomly 'hit' the button and stop
the players for the same test of hits.  If the players tap early, they have the chance to kill a monster
before it even attacks, so this encourages quick attacks, but still precise ones.  This continues until
one side is dead.

The job hunt eventually took precedence as the money ran tight, so this project slowed to a stop.
Here is the state of the game as it is now.  It is likely to resume.

*Long Live Dungeon Reincarnate!*

<img width="603" height="628" alt="Screenshot 2025-07-08 at 11 45 36 AM" src="https://github.com/user-attachments/assets/59f784f1-865a-4ca1-a488-34572e54d91d" />

<img width="602" height="629" alt="Screenshot 2025-07-08 at 11 46 19 AM" src="https://github.com/user-attachments/assets/46e0c3c6-7fa9-4d65-be5f-3dd979c79761" />

<img width="599" height="626" alt="Screenshot 2025-07-08 at 11 47 15 AM" src="https://github.com/user-attachments/assets/b2b60ee5-a1a4-4f74-8758-3f42a00039c4" />

<img width="601" height="631" alt="Screenshot 2025-07-08 at 11 48 03 AM" src="https://github.com/user-attachments/assets/660f47b0-da8d-4dc8-811a-4312be35b983" />

<img width="602" height="629" alt="Screenshot 2025-07-13 at 3 25 29 PM" src="https://github.com/user-attachments/assets/d364ec37-38c5-4fdf-b0b4-16a0b9357984" />


