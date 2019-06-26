//#include <iostream>
//#include "bitboard.h"
//#include "position.h"
#include "search.h"
#include "thread.h"
#include "tt.h"
#include "usi.h"

// ----------------------------------------
//  main()
// ----------------------------------------

int main(int argc, char* argv[])
{
	// --- �S�̓I�ȏ�����
	USI::init(Options);
	Bitboards::init();
	Position::init();
	Search::init();
	Threads.set(Options["Threads"]);
	//Search::clear();
	Eval::init();

	// USI�R�}���h�̉�����
	USI::loop(argc, argv);

	// �������āA�ҋ@�����Ă����X���b�h�̒�~
	Threads.set(0);

	return 0;
}
