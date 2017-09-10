#include <iostream>

#include "Scene.hpp"
#include "SimTime.hpp"

#undef main
int main(int argc, char * argv[])
{
	std::cout << "Oh heck!\n";
	Scene world;
	SimTime timer( 120.0f );
	while(!world.done())
	{
		timer.setCur();
		while( timer.getAcc() > 0.0 )
		{
			world.update( 0.05f );
			timer.incrAcc( -0.05 );
		}
		world.draw( timer.getAcc() );
	}

	return 0;
}
