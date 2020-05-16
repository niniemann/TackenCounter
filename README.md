> _**NOTE:** Although all screenshots were made in windows, this app is actually developed in linux and only cross-compiled for windows.
> You should have no problems compiling this yourself, with the only dependencies being [cereal](https://uscilab.github.io/cereal/) and Qt5.
> If you are a windows-user, look in the [releases](https://github.com/niniemann/TackenCounter/releases) section for pre-compiled stand-alone binaries._

## TackenCounter

What you found here is a small side project to keep track of the score in a game of "Doppelkopf".
When started, you are presented with a very minimalistic screen:

![](/img/tackencounter_0.png)

Whether you select to start a new or load an existing game, in both cases you are greeted by a
file dialog. The app automatically saves your changes on exit, and also after every little change made
(though with a 30 second delay to not spam too many writes on the disk).

Selecting a good font is quite difficult, especially when compatibility with linux **and** windows is wanted.
So... choose yourself.

The main screen is the "Log"-tab. Here you input the results of each round, by setting the base "Value",
ticking the box indicating if the round triggers "Bockrunden", and selecting which players won the round (green background).
BOCK-modifiers are included automatically in the score, as well as the different scoring of SOLO rounds.

![](/img/tackencounter_2.png)

You can choose if you want to play with 4 or 5 players. In the latter case, Bockrunden are extended to 5
(BOCK becomes BOCKY), and the indicator for the next player to skip the round is updated automatically (grey background).
The last column for the fifth player is never hidden -- in the 4-player-setting the player to skip a
round ajust always defaults to Player5.

Last but not least, the third tab provides some statistics on the game. I guess there are lots of possibilities to extend this. :)

![](/img/tackencounter_3.png)
