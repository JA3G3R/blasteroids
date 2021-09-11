---DESCRIPTION---

Just an ammateur implementation of the legendary arcade game blasteroids
-----------------

------------
  COMMANDS
------------

commands for any unix based users(mac users as well), just copy paste all of them in the terminal separated by a ; or run each of them individually:

sudo apt update --> update the system                                                                                           
sudo apt install liballegro5.2 liballegro5-dev allegro5-doc --> install the dependencies

git clone https://github.com/JA3G3R/blasteroids                                                                                 
cd blasteroids/

BELOW IS THE MAIN COMPILATION COMMAND [which produces the actual blasteroids game binary]

gcc -g blasteroids.c lib/* -lallegro -lallegro_font -lallegro_primitives -lm lpthread -o blasteroids

-------------------
<<<<<<< HEAD

=======
>>>>>>> 47d79889993b53e9f360bfc24dcdd951042a1f8c
