#include "../../config.h"

#if defined(YANEURAOU_ENGINE_DEEP)

#include "../../position.h"
#include "../../usi.h"
#include "../../thread.h"

#include "dlshogi_types.h"
#include "Node.h"
#include "UctSearch.h"
#include "dlshogi_searcher.h"

#include "../../eval/deep/nn_types.h"

// ��˂��牤�t���[�����[�N�ƁAdlshogi�̋��n�����s���R�[�h

// --- ��˂��牤��search��override

using namespace dlshogi;
using namespace Eval::dlshogi;

// �T�����{�́B�Ƃ�܁Aglobal�ɔz�u���Ă����B
DlshogiSearcher searcher;

void USI::extra_option(USI::OptionsMap& o)
{
	// �G���W���I�v�V�����𐶂₷
#if 0
    (*this)["Book_File"]                   = USIOption("book.bin");
    (*this)["Best_Book_Move"]              = USIOption(true);
    (*this)["OwnBook"]                     = USIOption(false);
    (*this)["Min_Book_Score"]              = USIOption(-3000, -ScoreInfinite, ScoreInfinite);
    (*this)["USI_Ponder"]                  = USIOption(false);
    (*this)["Stochastic_Ponder"]           = USIOption(true);
    (*this)["Time_Margin"]                 = USIOption(1000, 0, INT_MAX);
    (*this)["Mate_Root_Search"]            = USIOption(29, 0, 35);
    (*this)["DfPn_Hash"]                   = USIOption(2048, 64, 4096); // DfPn�n�b�V���T�C�Y
    (*this)["DfPn_Min_Search_Millisecs"]   = USIOption(300, 0, INT_MAX);
#endif

#ifdef MAKE_BOOK
	// ��Ղ𐶐�����Ƃ���PV�o�͂͗}�������ق����ǂ����B
    o["PV_Interval"]                 << USI::Option(0, 0, INT_MAX);
    o["Save_Book_Interval"]          << USI::Option(100, 0, INT_MAX);
#else
    o["PV_Interval"]                 << USI::Option(500, 0, INT_MAX);
#endif // !MAKE_BOOK

	o["UCT_NodeLimit"]				 << USI::Option(10000000, 100000, 1000000000); // UCT�m�[�h�̏��

	// �f�o�b�O�p�̃��b�Z�[�W�o�̗͂L��
	o["DebugMessage"]                << USI::Option(false);

	// �m�[�h���ė��p���邩�B
    o["ReuseSubtree"]                << USI::Option(true);

	// �����l : 1000������
	o["Resign_Threshold"]            << USI::Option(0, 0, 1000);

	// ���������̎��̒l : 1000������

	o["Draw_Value_Black"]            << USI::Option(500, 0, 1000);
    o["Draw_Value_White"]            << USI::Option(500, 0, 1000);

	// ���ꂪtrue�ł���Ȃ�Aroot color(�T���J�n�ǖʂ̎��)�����Ȃ�A
	//
	// Draw_Value_Black��Draw_Value_White�̒l�����ւ������̂Ƃ݂Ȃ��B
	// ���ł́u(��������肩��肩�͂킩��Ȃ�����)�����͂ł���ΐ�����_�������āA
	// ����̃\�t�g�͐����������������Ƃ݂Ȃ��Ă���v�󋵂ł́Aroot color(�J�n�ǖʂ̎��)�ƁA
	// root color�̔��΂̎��(�����color)�ɑ΂��āA���ꂼ��A0.7 , 0.5�̂悤�ɐݒ肵�������Ƃ�����B
	// ������������邽�߂ɁAroot color�����Ȃ�ADraw_Value_Black��Draw_Value_White�����ւ��Ă����
	// �I�v�V����������Ηǂ��B���ꂪ����B

	o["Draw_Value_From_Black"]       << USI::Option(false);

	// --- PUCT�̎��̒萔
	// ����A�T���p�����[�^�[�̈��ƍl�����邩��A�œK�Ȓl�����O�Ƀ`���[�j���O���Đݒ肷��悤��
	// ���Ă����A���[�U�[����͐G��Ȃ�(�G��Ȃ��Ă��ǂ�)�悤�ɂ��Ă����B
	// ���@dlshogi��optimizer�ōœK�����邽�߂ɊO�������Ă���悤���B

#if 0
	// fpu_reduction�̒l��100�����Őݒ�B
	// c_fpu_reduction_root�́Aroot�ł�fpu_reduction�̒l�B
    o["C_fpu_reduction"]             << USI::Option(27, 0, 100);
    o["C_fpu_reduction_root"]        << USI::Option(0, 0, 100);

    o["C_init"]                      << USI::Option(144, 0, 500);
    o["C_base"]                      << USI::Option(28288, 10000, 100000);
    o["C_init_root"]                 << USI::Option(116, 0, 500);
    o["C_base_root"]                 << USI::Option(25617, 10000, 100000);

    o["Softmax_Temperature"]         << USI::Option(174, 1, 500);
#endif

	// �eGPU�p��DNN���f�����ƁA����GPU�p��UCT�T���̃X���b�h���ƁA����GPU�Ɉ�x�ɉ��̋ǖʂ��܂Ƃ߂ĕ]��(���_)���s�킹��̂��B
	// GPU�͍ő��8�܂ň�����B

    o["UCT_Threads1"]                << USI::Option(4, 0, 256);
    o["UCT_Threads2"]                << USI::Option(0, 0, 256);
    o["UCT_Threads3"]                << USI::Option(0, 0, 256);
    o["UCT_Threads4"]                << USI::Option(0, 0, 256);
    o["UCT_Threads5"]                << USI::Option(0, 0, 256);
    o["UCT_Threads6"]                << USI::Option(0, 0, 256);
    o["UCT_Threads7"]                << USI::Option(0, 0, 256);
    o["UCT_Threads8"]                << USI::Option(0, 0, 256);
    o["DNN_Model1"]                  << USI::Option(R"(model.onnx)");
    o["DNN_Model2"]                  << USI::Option("");
    o["DNN_Model3"]                  << USI::Option("");
    o["DNN_Model4"]                  << USI::Option("");
    o["DNN_Model5"]                  << USI::Option("");
    o["DNN_Model6"]                  << USI::Option("");
    o["DNN_Model7"]                  << USI::Option("");
    o["DNN_Model8"]                  << USI::Option("");

#if defined(ONNXRUNTIME)
	// CPU���g���Ă��邱�Ƃ�����̂ŁAdefault�l�A������Ə��Ȃ߂ɂ��Ă����B
	o["DNN_Batch_Size1"]             << USI::Option(32, 1, 1024);
#else
	// �ʏ펞�̐���128 , �����̎��͐���256�B
	o["DNN_Batch_Size1"]             << USI::Option(128, 1, 1024);
#endif
	o["DNN_Batch_Size2"]             << USI::Option(0, 0, 65536);
    o["DNN_Batch_Size3"]             << USI::Option(0, 0, 65536);
    o["DNN_Batch_Size4"]             << USI::Option(0, 0, 65536);
    o["DNN_Batch_Size5"]             << USI::Option(0, 0, 65536);
    o["DNN_Batch_Size6"]             << USI::Option(0, 0, 65536);
    o["DNN_Batch_Size7"]             << USI::Option(0, 0, 65536);
    o["DNN_Batch_Size8"]             << USI::Option(0, 0, 65536);


    //(*this)["Const_Playout"]               = USIOption(0, 0, INT_MAX);
	// ���@Playout���Œ�B�����NodeLimit�łł���̂ŕs�v�B

}

// "isready"�R�}���h�ɑ΂��鏉�񉞓�
void Search::init(){}

// "isready"�R�}���h���ɖ���Ăяo�����B
void Search::clear()
{
	// �G���W���I�v�V�����̔��f
#if 0
			// �I�v�V�����ݒ�
			dfpn_min_search_millisecs = options["DfPn_Min_Search_Millisecs"];
#endif

	searcher.SetPvInterval((TimePoint)Options["PV_Interval"]);

	// �m�[�h���ė��p���邩�̐ݒ�B
	searcher.SetReuseSubtree(Options["ReuseSubtree"]);

	// �����l
	searcher.SetResignThreshold((int)Options["Resign_Threshold"]);

	// �f�o�b�O�p�̃��b�Z�[�W�o�̗͂L���B
	searcher.debug_message = Options["DebugMessage"];

	// �X���b�h���ƊeGPU��batchsize��searcher�ɐݒ肷��B

	const int new_thread[max_gpu] = {
		(int)Options["UCT_Threads1"], (int)Options["UCT_Threads2"], (int)Options["UCT_Threads3"], (int)Options["UCT_Threads4"],
		(int)Options["UCT_Threads5"], (int)Options["UCT_Threads6"], (int)Options["UCT_Threads7"], (int)Options["UCT_Threads8"]
	};
	const int new_policy_value_batch_maxsize[max_gpu] = {
		(int)Options["DNN_Batch_Size1"], (int)Options["DNN_Batch_Size2"], (int)Options["DNN_Batch_Size3"], (int)Options["DNN_Batch_Size4"],
		(int)Options["DNN_Batch_Size5"], (int)Options["DNN_Batch_Size6"], (int)Options["DNN_Batch_Size7"], (int)Options["DNN_Batch_Size8"]
	};

	std::vector<int> thread_nums;
	std::vector<int> policy_value_batch_maxsizes;
	for (int i = 0; i < max_gpu; ++i)
	{
		thread_nums.push_back(new_thread[i]);
		policy_value_batch_maxsizes.push_back(new_policy_value_batch_maxsize[i]);
	}
	searcher.SetThread(thread_nums, policy_value_batch_maxsizes);

	// ���̑��Adlshogi�ɂ͂��邯�ǁA�T�|�[�g���Ȃ����́B

	// UCT_NodeLimit ���@dlshogi�ł͑��݂��邪�A��˂��牤�������炠��G���W���I�v�V������"NodesLimit"�𗬗p���ėǂ��Ǝv���B
	// EvalDir�@�@�@ ���@dlshogi�ł̓T�|�[�g����Ă��Ȃ����A��˂��牤�́AEvalDir�ɂ��郂�f���t�@�C����ǂݍ��ނ悤�ɂ���B

	// �ȉ����A�T���p�����[�^�[������A����Ȃ��B�J�������œK�l�Ƀ`���[�j���O���ׂ��Ƃ����l���B
	// C_fpu_reduction , C_fpu_reduction_root , C_init , C_base , C_init_root , C_base_root
#if 0
	search_options.c_fpu_reduction      =                Options["C_fpu_reduction"     ] / 100.0f;
	search_options.c_fpu_reduction_root =                Options["C_fpu_reduction_root"] / 100.0f;

	search_options.c_init               =                Options["C_init"              ] / 100.0f;
	search_options.c_base               = (NodeCountType)Options["C_base"              ];
	search_options.c_init_root          =                Options["C_init_root"         ] / 100.0f;
	search_options.c_base_root          = (NodeCountType)Options["C_base_root"         ];

	set_softmax_temperature(options["Softmax_Temperature"] / 100.0f);
#endif

	// softmax�̎��̃{���c�}�����x�ݒ�
	// ����́Adlshogi��"Softmax_Temperature"�̒l�B(174)
	// ���ߑł��ł����Ǝv���B
	Eval::dlshogi::set_softmax_temperature( 174 / 100.0f);

	searcher.SetDrawValue(
		(int)Options["Draw_Value_Black"],
		(int)Options["Draw_Value_White"],
		Options["Draw_Value_From_Black"]);

	searcher.SetPonderingMode(Options["USI_Ponder"]);

	searcher.InitializeUctSearch((NodeCountType)Options["UCT_NodeLimit"]);

	searcher.SetModelPaths(Eval::dlshogi::ModelPaths);

#if 0
	// dlshogi�ł́A
	// "isready"�ɑ΂���node limit = 1 , batch_size = 128 �ŒT�����Ă����B
	// �����ǖʂɑ΂��Ă͂��Ɠ����H

	// ����T�����L���b�V��
	Position pos_tmp;
	StateInfo si;
	pos_tmp.set_hirate(&si,Threads.main());
	LimitsType limits;
	limits.nodes = 1;
	searcher.SetLimits(limits);
	Move ponder;
	auto start_threads = [&]() {
		searcher.parallel_search(pos_tmp,0);
	};
	searcher.UctSearchGenmove(&pos_tmp, pos_tmp.sfen(), {}, ponder , false, start_threads);
#endif

}

// "go"�R�}���h�ɑ΂��ČĂяo�����B
void MainThread::search()
{
	// �J�n�ǖʂ̎�Ԃ�global�Ɋi�[���Ă������ق����֗��B
	searcher.search_limit.root_color = rootPos.side_to_move();

	// "NodesLimit"�̒l�ȂǁA�����"go"�R�}���h�ɂ���Č��肵���l�����f�����B
	searcher.SetLimits(Search::Limits);

	// MultiPV
	// ���@dlshogi�ł͌��󖢃T�|�[�g�����A�~�����̂Œǉ����Ă����B
	// ����́Aisready�̂��ƁAgo�̒��O�܂ŕύX�\
	searcher.search_options.multi_pv = Options["MultiPV"];

	// �S�X���b�h�ŒT�����J�n����lambda
	auto start_threads = [&]()
	{
		Threads.start_searching(); // main�ȊO��thread���J�n����
		Thread::search();          // main thread(���̃X���b�h)���T���ɎQ������B
	};

	Move ponderMove;
	Move move = searcher.UctSearchGenmove(&rootPos, rootPos.sfen(),{}, ponderMove, ponder , start_threads);

	// �ő�depth�[���ɓ��B�����Ƃ��ɁA�����܂Ŏ��s�����B���邪�A
	// �܂�Threads.stop�������Ă��Ȃ��B�������Aponder����Ago infinite�ɂ��T���̏ꍇ�A
	// USI(UCI)�v���g�R���ł́A"stop"��"ponderhit"�R�}���h��GUI���瑗���Ă���܂�best move���o�͂��Ă͂Ȃ�Ȃ��B
	// ����䂦�A�P�ɂ�����GUI���炻���̂����ꂩ�̃R�}���h�������Ă���܂ő҂B
	// "stop"�������Ă�����Threads.stop == true�ɂȂ�B
	// "ponderhit"�������Ă�����Threads.ponder == false�ɂȂ�̂ŁA�����҂B(stopOnPonderhit�͗p���Ȃ�)
	// "go infinite"�ɑ΂��Ă�stop�������Ă���܂ő҂B
	// ���Ȃ݂�Stockfish�̂ق��A�����̃R�[�h�ɒ��炭������̃o�O���������B
	// ��˂��牤�̂ق��́A���Ȃ葁�����炱�̍\���ŏ����Ă����B�ŋ߂�Stockfish�ł͂��̏������ɒǐ������B
	while (!Threads.stop && (this->ponder || Search::Limits.infinite))
	{
		//	������̎v�l�͏I����Ă���킯������A������x�ׂ����҂��Ă����Ȃ��B
		// (�v�l�̂��߂ɂ͌v�Z�������g���Ă��Ȃ��̂ŁB)
		Tools::sleep(1);

		// Stockfish�̃R�[�h�A�����Abusy wait�ɂȂ��Ă��邪�A�������ɂ���͗ǂ��Ȃ��Ǝv���B
	}

	// �S�X���b�h�ɒ�~���߂𑗂�B
	Threads.stop = true;

	// �e�X���b�h���I������̂�ҋ@����(�J�n���Ă��Ȃ���΂��Ȃ��ō\��Ȃ�)
	Threads.wait_for_search_finished();


	sync_cout << "bestmove " << to_usi_string(move);

	// USI_Ponder��true�Ȃ�΁B
	if (searcher.search_options.usi_ponder && ponderMove)
		std::cout << " ponder " << to_usi_string(move);

	std::cout << sync_endl;
}

void Thread::search()
{
	// searcher���A���̃X���b�h���ǂ̃C���X�^���X��
	// UCTSearcher::ParallelUctSearch()���Ăяo�����m���Ă���B

	// ����rootPos�̓X���b�h���Ƃɗp�ӂ���Ă��邩��R�s�[�\�B

	searcher.parallel_search(rootPos,thread_id());
}

//MainThread::~MainThread()
//{
//	searcher.FinalizeUctSearch();
//}


#endif // defined(YANEURAOU_ENGINE_DEEP)
