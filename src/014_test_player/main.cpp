#include <iostream>
#include "player.h"
int main(int argc, char *argv[])
{
	Player play;
	play.Open("疾速追杀.mp4",nullptr);
	play.Start();
	for (;;)
	{
		play.Update();
		MSleep(10);
	}
	getchar();
	return 0;
}
