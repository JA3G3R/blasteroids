---DESCRIPTION---

Just an ammateur implementation of the legendary arcade game blasteroids
-----------------

------------
  COMMANDS
------------

commands for any unix based users(mac users as well), just copy paste all of them in the terminal separated by a ; or run each of them individually:

sudo apt update --> update the system                                                                                           
sudo apt install liballegro4.4 liballegro4-dev allegro4-doc --> install the dependencies

git clone https://github.com/JA3G3R/blasteroids                                                                                 
cd blasteroids/

BELOW IS THE MAIN COMPILATION COMMAND [which produces the actual blasteroids game binary]

gcc -g blasteroids.c lib/prototype_funcs.c lib/collision_detection.c lib/sp_collision.c lib/menu_functions.c lib/translate_asteroid.c lib/translate_funcs.c -lallegro -lallegro_font -lallegro_primitives -o blasteroids

-------------------
