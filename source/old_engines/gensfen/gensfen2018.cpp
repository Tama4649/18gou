//
// �J�����̋��t�ǖʂ̎�������
//

// ���݂̕]���֐��ŁA�Ȃ�ׂ������m���̍��������I�ȋǖʂł��A�`�����݊p�ɋ߂��ǖʂ����t�����̊J�n�ǖʂƂ��ėp����B


#include "../shogi.h"

#if defined(EVAL_LEARN) && defined(USE_GENSFEN2018)

#include <sstream>
using namespace std;

namespace Learner {

	// -----------------------------------
	//  �����𐶐�����worker(�X���b�h����)
	// -----------------------------------
//	const static int GENSFEN_MULTI_PV = 24; // 1)
	const static int GENSFEN_MULTI_PV = 8;  // 2)

	// �K��ς�node�Ɋւ�������L�^���Ă����\���́B
	// ����node�ŁAMultiPV�ŒT�������Ƃ��̂悳���Ȏw������8����L�^���Ă����B
	struct NodeInfo
	{
		u16 moves[GENSFEN_MULTI_PV];	// �w����(�ő�8��)
		u32 length;						// �w���肪���肠�邩�B
		s32 game_ply;					// game ply���قȂ�Ȃ�z���Ă�̂����m��Ȃ��̂œK�p���Ȃ��B
		u64 key;						// �ǖʂ�hash key  
		// 1) = 48+4+4+8 = 64bytes
		// 2) = 16+4+4+8 = 32bytes
	};

	// �����X���b�h��sfen�𐶐����邽�߂̃N���X
	struct MultiThinkGenSfen2018 : public MultiThink
	{
		// hash_size : NodeInfo���i�[���邽�߂�hash size���w�肷��B�P�ʂ�[MB]�B
		// �������ɗ]�T������Ȃ�傫�߂̒l���w�肷��̂��D�܂����B
		MultiThinkGenSfen2018(int search_depth_, int search_depth2_, SfenWriter& sw_ , u64 hash_size)
			: search_depth(search_depth_), search_depth2(search_depth2_), sw(sw_)
		{
			hash.resize(GENSFEN_HASH_SIZE);

			node_info_size = (hash_size * (u64)1024 * (u64)1024) / (u64)sizeof(NodeInfo);
			node_info_hash.resize(node_info_size);

			// PC����񉻂���gensfen����Ƃ��ɓ�������seed�������Ă��Ȃ����m�F�p�̏o�́B
			std::cout << endl << prng << std::endl;
		}

		virtual void thread_worker(size_t thread_id);
		void start_file_write_worker() { sw.start_file_write_worker(); }

		//  search_depth = �ʏ�T���̒T���[��
		int search_depth;
		int search_depth2;

		// ��������ǖʂ̕]���l�̏��
		int eval_limit;

		// �����o���ǖʂ�ply(�����ǖʂ���̎萔)�̍ŏ��A�ő�B
		int write_minply;
		int write_maxply;

		// sfen�̏����o����
		SfenWriter& sw;

		// ����ǖʂ̏����o���𐧌����邽�߂�hash
		// hash_index�����߂邽�߂�mask�Ɏg���̂ŁA2**N�łȂ���΂Ȃ�Ȃ��B
		vector<NodeInfo> node_info_hash;
		u64 node_info_size;

		// node_info_hash��node���������o������(��������)
		atomic<u64> node_info_count;

		// ����ǖʂ̏����o���𐧌����邽�߂�hash
		// hash_index�����߂邽�߂�mask�Ɏg���̂ŁA2**N�łȂ���΂Ȃ�Ȃ��B
		static const u64 GENSFEN_HASH_SIZE = 64 * 1024 * 1024;

		vector<Key> hash; // 64MB*sizeof(HASH_KEY) = 512MB
	};

	//  thread_id    = 0..Threads.size()-1
	void MultiThinkGenSfen2018::thread_worker(size_t thread_id)
	{
		// �Ƃ肠�����A�����o���萔�̍ő�̂Ƃ���ň������������ɂȂ���̂Ƃ���B
		const int MAX_PLY2 = write_maxply;

		// StateInfo���ő�萔�� + Search��PV��leaf�ɂ܂Ői�߂�buffer
		std::vector<StateInfo, AlignedAllocator<StateInfo>> states(MAX_PLY2 + 50 /* == search_depth + �� */);
		StateInfo si;

		// ����̎w����B���̎w����ŋǖʂ�i�߂�B
		Move m = MOVE_NONE;

		// �I���t���O
		bool quit = false;

		// �K��񐔉�ɂȂ�܂ŌJ��Ԃ�
		while (!quit)
		{
			// Position�ɑ΂��ď]���X���b�h�̐ݒ肪�K�v�B
			// ���񉻂���Ƃ��́AThreads (���ꂪ���̂� vector<Thread*>�Ȃ̂ŁA
			// Threads[0]...Threads[thread_num-1]�܂łɑ΂��ē����悤�ɂ���Ηǂ��B
			auto th = Threads[thread_id];

			auto& pos = th->rootPos;
			pos.set_hirate(&si, th);

			// 1�Ǖ��̋ǖʂ�ۑ����Ă����A�I�ǂ̂Ƃ��ɏ��s���܂߂ď����o���B
			// �����o���֐��́A���̉��ɂ���flush_psv()�ł���B
			PSVector a_psv;
			a_psv.reserve(MAX_PLY2 + 50);

			// a_psv�ɐς܂�Ă���ǖʂ��t�@�C���ɏ����o���B
			// lastTurnIsWin : a_psv�ɐς܂�Ă���ŏI�ǖʂ̎��̋ǖʂł̏��s
			// �����̂Ƃ���1�B�����̂Ƃ���-1�B���������̂Ƃ���0��n���B
			// �Ԃ��l : �����K��ǖʐ��ɒB�����̂ŏI������ꍇ��true�B
			auto flush_psv = [&](s8 lastTurnIsWin)
			{
				s8 isWin = lastTurnIsWin;

				// �I�ǂ̋ǖ�(�̈�O)���珉��Ɍ����āA�e�ǖʂɊւ��āA�΋ǂ̏��s�̏���t�^���Ă����B
				// a_psv�ɕۑ�����Ă���ǖʂ�(��ԓI��)�A�����Ă�����̂Ƃ���B
				for (auto it = a_psv.rbegin(); it != a_psv.rend(); ++it)
				{
					// isWin == 0(��������)�Ȃ� -1���|���Ă� 0(��������)�̂܂�
					isWin = -isWin;
					it->game_result = isWin;

					// �ǖʂ������o�����Ǝv������K��񐔂ɒB���Ă����B
					// get_next_loop_count()���ŃJ�E���^�[�����Z����̂�
					// �ǖʂ��o�͂����Ƃ��ɂ�����Ăяo���Ȃ��ƃJ�E���^�[�������B
					auto loop_count = get_next_loop_count();
					if (loop_count == UINT64_MAX)
					{
						// �I���t���O�𗧂ĂĂ����B
						quit = true;
						return;
					}

					// 2�̗ݏ�ł���Ȃ�node_info_count�̐����o�́B
					if (POPCNT64(loop_count) == 1)
					{
						sync_cout << endl << "loop_count = " << loop_count << " , node_info_count = " << node_info_count << sync_endl;

#if 0
						// 1�`32��ڂ܂ł�node���ǂꂭ�炢�i�[����Ă��邩�o�͂���B
						for (int i = 1; i <= 32; ++i)
						{
							u64 c = 0;
							for (u64 j = 0; j < node_info_size; ++j)
								if (node_info_hash[j].game_ply == i)
									c++;
							if (c)
								sync_cout << "PLY = " << i << " , nodes = " << c << sync_endl;
						}
#endif
					}

					// �ǖʂ�������o���B
					sw.write(thread_id, *it);

#if 0
					pos.set_from_packed_sfen(it->sfen);
					cout << pos << "Win : " << it->isWin << " , " << it->score << endl;
#endif
				}
			};

			// node_info_hash�𒲂ׂ�̂��̃t���O
			// �����ǖʂ���Anode_info_hash��hit�����������͒��ׂ�B
			bool opening = true;

			// ply : �����ǖʂ���̎萔
			for (int ply = 0; ply < MAX_PLY2; ++ply)
			{
				//cout << pos << endl;

				// ����̒T��depth
				// goto�Ŕ�Ԃ̂Ő�ɐ錾���Ă����B
				int depth = search_depth + (int)prng.rand(search_depth2 - search_depth + 1);

				// �S���ċl��ł����肵�Ȃ����H
				if (pos.is_mated())
				{
					// (���̋ǖʂ̈�O�̋ǖʂ܂ł͏����o��)
					flush_psv(-1);
					break;
				}

				// �錾����
				if (pos.DeclarationWin() != MOVE_NONE)
				{
					// (���̋ǖʂ̈�O�̋ǖʂ܂ł͏����o��)
					flush_psv(1);
					break;
				}

				// �K��ς�node�ł���Ȃ�A���̂Ȃ����烉���_���Ɉ�w�����I�сA�i�߂�B
				if (opening)
				{
					auto key = pos.key();
					auto hash_index = (size_t)(key & (node_info_size - 1));
					auto& entry = node_info_hash[hash_index];

					// ����node�Ƌǖʂ�hash key���҂������v�����Ȃ�..
					if (key == entry.key)
					{
						// �ǖʁA�����炭�z��������Ă�̂ŏI���B
						if (pos.game_ply() > entry.game_ply)
							break;

						const int length = entry.length;
						if (length)
						{
							// �O��MultiPV�ŒT�������Ƃ��̎w����̂����A�悳���Ȃ��̂������_���Ɏw�������I��
							u16 move16 = entry.moves[prng.rand(length)];
							if (move16)
							{
								m = pos.move16_to_move((Move)move16);
								if (pos.pseudo_legal(m) && pos.legal(m))
								{
									// �w�����m�ɓ����Ă���̂ŁA�����do_move()���Ă��܂��B
									goto DO_MOVE;
								}
							}
						}
					}

					// �ǂ������̋ǖʗp�̏�񂪏������܂�Ă��Ȃ��悤�Ȃ̂ŁAMultiPV�ŒT�����Ă���Node�̏�����������ł��܂��B

					// 32��ڈȍ~�ł���Ȃ�A����node����͕��ʂɑ΋ǃV�~�����[�V�������s�Ȃ��B
					// node_info_hash�𒲂ׂ�K�v���Ȃ��B
					if (ply >= 32)
						opening = false;

					Learner::search(pos, depth, GENSFEN_MULTI_PV);
					// rootMoves�̏��N��̂Ȃ������I��

					auto& rm = pos.this_thread()->rootMoves;

					u64 length = min((u64)rm.size(), (u64)GENSFEN_MULTI_PV);
					int count = 0;
					u16 moves[GENSFEN_MULTI_PV];
					for (u64 i = 0; i < length /* && count < GENSFEN_MULTI_PV */ ; ++i)
					{
						auto value = rm[i].score;

						// �݊p����������Ȃ�w����͏��O�B(bestmove�����ꂾ�Ƃ�����A������������node�̒��O��node�����������̂����c)
						if (value < -400 || 400 < value)
							continue;

						moves[count++] = (u16)rm[i].pv[0];
					}
					if (count)
					{
						for (int i = 0; i < count; ++i)
							entry.moves[i] = moves[i];
						entry.game_ply = pos.game_ply();
						entry.length = count;
						entry.key = key;
						node_info_count.fetch_add(1, std::memory_order_relaxed);

						// �����_����1��I��ł���Ői�߂�
						m = pos.move16_to_move((Move)moves[prng.rand(count)]);
						ASSERT_LV3(pos.pseudo_legal(m) && pos.legal(m));

						// �w�����m�ɓ����Ă���̂ŁA�����do_move()���Ă��܂��B
						goto DO_MOVE;
					}

					// �݊p�̋ǖʂł͂Ȃ��̂ŏ��Ղ͔������Ƃ��������B
					opening = false;
				}

				// -- ���ʂɒT�����Ă��̎w����ŋǖʂ�i�߂�B

				{
					// �u���\�̐���J�E���^�[��i�߂Ă����Ȃ���
					// �����ǖʎ��ӂ�hash�Փ˂���TTEntry�ɓ�����A�ςȕ]���l���E���Ă��āA
					// eval_limit���Ⴂ�Ƃ���������ďI�����Ă��܂��̂ŁA���܂ł����t�ǖʂ���������Ȃ��Ȃ�B
					// �u���\���̂́A�X���b�h���Ƃɕێ����Ă���̂ŁA������TT.new_search()���Ăяo���Ė��Ȃ��B

					// �]���l�̐�Βl�����̒l�ȏ�̋ǖʂɂ��Ă�
					// ���̋ǖʂ��w�K�Ɏg���̂͂��܂�Ӗ����Ȃ��̂ł��̎������I������B
					// ����������ď��s�������Ƃ�������������B
					TT.new_search();

					auto pv_value1 = search(pos, depth);

					auto value1 = pv_value1.first;
					auto& pv1 = pv_value1.second;

					// 1��l�߁A�錾�����Ȃ�΁A������mate_in(2)���Ԃ�̂�eval_limit�̏���l�Ɠ����l�ɂȂ�A
					// ����if���͕K���^�ɂȂ�Bresign�ɂ��Ă����l�B

					if (abs(value1) >= eval_limit)
					{
						//					sync_cout << pos << "eval limit = " << eval_limit << " over , move = " << pv1[0] << sync_endl;

						// ���̋ǖʂ�value1 >= eval_limit�Ȃ�΁A(���̋ǖʂ̎�ԑ���)�����ł���B
						flush_psv((value1 >= eval_limit) ? 1 : -1);
						break;
					}

					// �������Ȏw����̌���
					if (pv1.size() > 0
						&& (pv1[0] == MOVE_RESIGN || pv1[0] == MOVE_WIN || pv1[0] == MOVE_NONE)
						)
					{
						// MOVE_WIN�́A���̎�O�Ő錾�����̋ǖʂł��邩�`�F�b�N���Ă���̂�
						// �����Ő錾�����̎w���肪�Ԃ��Ă��邱�Ƃ͂Ȃ��͂��B
						// �܂��AMOVE_RESIGN�̂Ƃ�value1��1��l�߂̃X�R�A�ł���Aeval_limit�̍ŏ��l(-31998)�̂͂��Ȃ̂����c�B
						cout << "Error! : " << pos.sfen() << m << value1 << endl;
						break;
					}

					// �e�����ɉ����������B

					s8 is_win = 0;
					bool game_end = false;
					auto draw_type = pos.is_repetition(0);
					switch (draw_type)
					{
					case REPETITION_WIN: is_win = 1; game_end = true; break;
					case REPETITION_DRAW: is_win = 0; game_end = true; break;
					case REPETITION_LOSE: is_win = -1; game_end = true; break;

						// case REPETITION_SUPERIOR: break;
						// case REPETITION_INFERIOR: break;
						// �����͈Ӗ�������̂Ŗ������ėǂ��B
					default: break;
					}

					if (game_end)
					{
						break;
					}

					// PV�̎w�����leaf node�܂Ői�߂āA����leaf node��evaluate()���Ăяo�����l��p����B
					auto evaluate_leaf = [&](Position& pos, vector<Move>& pv)
					{
						auto rootColor = pos.side_to_move();

						int ply2 = ply;
						for (auto m : pv)
						{
							// �f�o�b�O�p�̌��؂Ƃ��āA�r���ɔ񍇖@�肪���݂��Ȃ����Ƃ��m�F����B
							// NULL_MOVE�͂��Ȃ����̂Ƃ���B

							// �\���Ƀe�X�g�����̂ŃR�����g�A�E�g�ŗǂ��B
#if 1
						// �񍇖@��͂���Ă��Ȃ��͂��Ȃ̂����B
						// �錾������mated()�łȂ����Ƃ͏�Ńe�X�g���Ă���̂�
						// �ǂ݋؂Ƃ���MOVE_WIN��MOVE_RESIGN�����Ȃ����Ƃ͕ۏ؂���Ă���B(�͂������c)
							if (!pos.pseudo_legal(m) || !pos.legal(m))
							{
								cout << "Error! : " << pos.sfen() << m << endl;
							}
#endif
							pos.do_move(m, states[ply2++]);

							// ���m�[�hevaluate()���Ăяo���Ȃ��ƁAevaluate()�̍����v�Z���o���Ȃ��̂Œ��ӁI
							// depth��8�ȏゾ�Ƃ��̍����v�Z�͂��Ȃ��ق��������Ǝv����B
							if (depth < 8)
								Eval::evaluate_with_no_return(pos);
						}

						// leaf�ɓ��B
						//      cout << pos;

						auto v = Eval::evaluate(pos);
						// evaluate()�͎�ԑ��̕]���l��Ԃ��̂ŁA
						// root_color�ƈႤ��ԂȂ�Av�𔽓]�����ĕԂ��Ȃ��Ƃ����Ȃ��B
						if (rootColor != pos.side_to_move())
							v = -v;

						// �����߂��B
						// C++x14�ɂ��Ȃ��āA���܂�reverse�ŉ�foreach����Ȃ��̂��c�B
						//  for (auto it : boost::adaptors::reverse(pv))

						for (auto it = pv.rbegin(); it != pv.rend(); ++it)
							pos.undo_move(*it);

						return v;
					};

					// depth 0�̏ꍇ�Apv�������Ă��Ȃ��̂�depth 2�ŒT�����Ȃ����B
					if (search_depth <= 0)
					{
						pv_value1 = search(pos, 2);
						pv1 = pv_value1.second;
					}

					// �����ǖʎ��ӂ͂͗ގ��ǖʂ΂���Ȃ̂�
					// �w�K�ɗp����Ɖߊw�K�ɂȂ肩�˂Ȃ����珑���o���Ȃ��B
					// ���@��r�������ׂ�
					if (ply < write_minply - 1)
					{
						a_psv.clear();
						goto SKIP_SAVE;
					}

					// ����ǖʂ������o�����Ƃ��납�H
					// ����A������PC�ŕ��񂵂Đ������Ă���Ɠ����ǖʂ��܂܂�邱�Ƃ�����̂�
					// �ǂݍ��݂̂Ƃ��ɂ����l�̏����������ق����ǂ��B
					{
						auto key = pos.key();
						auto hash_index = (size_t)(key & (GENSFEN_HASH_SIZE - 1));
						auto key2 = hash[hash_index];
						if (key == key2)
						{
							// �X�L�b�v����Ƃ��͂���ȑO�Ɋւ���
							// ���s�̏�񂪂��������Ȃ�̂ŕۑ����Ă���ǖʂ��N���A����B
							// �ǂ݂̂��Ahash�����v�������_�ł����ȑO�̋ǖʂ����v���Ă���\������������
							// �����o�����l���Ȃ��B
							a_psv.clear();
							goto SKIP_SAVE;
						}
						hash[hash_index] = key; // �����key�ɓ���ւ��Ă����B
					}

					// �ǖʂ̈ꎞ�ۑ��B
					{
						a_psv.emplace_back(PackedSfenValue());
						auto &psv = a_psv.back();

						// pack��v������Ă���Ȃ�pack���ꂽsfen�Ƃ��̂Ƃ��̕]���l�������o���B
						// �ŏI�I�ȏ����o���́A���s�����Ă���B
						pos.sfen_pack(psv.sfen);

						// PV line��leaf node�ł�root color���猩��evaluate()�̒l���擾�B
						// search()�̕Ԃ��l�����̂܂܎g���̂Ƃ�������̂Ƃ̑P���͗ǂ��킩��Ȃ��B
						psv.score = evaluate_leaf(pos, pv1);
						psv.gamePly = ply;

						// PV�̏�������o���B�����depth 0�łȂ�����͑��݂���͂��B
						ASSERT_LV3(pv_value1.second.size() >= 1);
						Move pv_move1 = pv_value1.second[0];
						psv.move = pv_move1;
					}

				SKIP_SAVE:;

					// ���̂�PV�������Ȃ�����(�u���\�Ȃǂ�hit���ċl��ł����H)�̂Ŏ��̑΋ǂɍs���B
					// ���Ȃ�̃��A�P�[�X�Ȃ̂Ŗ������ėǂ��Ǝv���B
					if (pv1.size() == 0)
						break;

					// search_depth��ǂ݂̎w����ŋǖʂ�i�߂�B
					m = pv1[0];
				}

			DO_MOVE:;
				pos.do_move(m, states[ply]);

				// �����v�Z���s�Ȃ����߂ɖ�node evaluate()���Ăяo���Ă����B
				Eval::evaluate_with_no_return(pos);

			} // for (int ply = 0; ply < MAX_PLY2 ; ++ply)

		} // while(!quit)

		sw.finalize(thread_id);
	}


	// gensfen2018�R�}���h�{��
	void gen_sfen2018(Position& pos, istringstream& is)
	{
		// �X���b�h��(����́AUSI��setoption�ŗ^������)
		u32 thread_num = (u32)Options["Threads"];

		// ���������̌� default = 80���ǖ�(Ponanza�d�l)
		u64 loop_max = 8000000000UL;

		// �]���l�����̒l�ɂȂ����琶����ł��؂�B
		int eval_limit = 3000;

		// �T���[��
		int search_depth = 3;
		int search_depth2 = INT_MIN;

		// �����o���ǖʂ�ply(�����ǖʂ���̎萔)�̍ŏ��A�ő�B
		int write_minply = 16;
		int write_maxply = 400;

		// �����o���t�@�C����
		string output_file_name = "generated_kifu.bin";

		// NodeInfo�̂��߂�hash size
		// �P�ʂ�[MB] 2�̗ݏ�ł��邱�ƁB
		u64 node_info_hash_size = 2 * 1024; // 2GB

		string token;

		// eval hash��hit����Ə����ǖʕt�߂̕]���l�Ƃ��āAhash�Փ˂��đ傫�Ȓl���������܂�Ă��܂���
		// eval_limit���������ݒ肳��Ă���Ƃ��ɏ����ǖʂŖ���eval_limit�𒴂��Ă��܂��ǖʂ̐������i�܂Ȃ��Ȃ�B
		// ���̂��߁Aeval hash�͖���������K�v������B
		// ����eval hash��hash�Փ˂����Ƃ��ɁA�ςȒl�̕]���l���g���A��������t�Ɏg���̂��C���������Ƃ����̂�����B
		bool use_eval_hash = false;

		// ���̒P�ʂŃt�@�C���ɕۑ�����B
		// �t�@�C������ file_1.bin , file_2.bin�̂悤�ɘA�Ԃ����B
		u64 save_every = UINT64_MAX;

		// �t�@�C�����̖����Ƀ����_���Ȑ��l��t�^����B
		bool random_file_name = false;

		while (true)
		{
			token = "";
			is >> token;
			if (token == "")
				break;

			if (token == "depth")
				is >> search_depth;
			else if (token == "depth2")
				is >> search_depth2;
			else if (token == "loop")
				is >> loop_max;
			else if (token == "output_file_name")
				is >> output_file_name;
			else if (token == "eval_limit")
			{
				is >> eval_limit;
				// �ő�l��1��l�݂̃X�R�A�ɐ�������B(�������Ȃ��ƃ��[�v���I�����Ȃ��\��������̂�)
				eval_limit = std::min(eval_limit, (int)mate_in(2));
			}
			else if (token == "write_minply")
				is >> write_minply;
			else if (token == "write_maxply")
				is >> write_maxply;
			else if (token == "use_eval_hash")
				is >> use_eval_hash;
			else if (token == "save_every")
				is >> save_every;
			else if (token == "random_file_name")
				is >> random_file_name;
			else if (token == "node_info_hash_size")
				is >> node_info_hash_size;
			else
				cout << "Error! : Illegal token " << token << endl;
		}

#if defined(USE_GLOBAL_OPTIONS)
		// ���Ƃŕ������邽�߂ɕۑ����Ă����B
		auto oldGlobalOptions = GlobalOptions;
		GlobalOptions.use_eval_hash = use_eval_hash;
#endif

		// search depth2���ݒ肳��Ă��Ȃ��Ȃ�Asearch depth�Ɠ����ɂ��Ă����B
		if (search_depth2 == INT_MIN)
			search_depth2 = search_depth;

		if (random_file_name)
		{
			// output_file_name�ɂ��̎��_�Ń����_���Ȑ��l��t�^���Ă��܂��B
			PRNG r;
			// �O�̂��ߗ����U�蒼���Ă����B
			for (int i = 0; i<10; ++i)
				r.rand(1);
			auto to_hex = [](u64 u) {
				std::stringstream ss;
				ss << std::hex << u;
				return ss.str();
			};
			// 64bit�̐��l�ŋ��R���Ԃ�ƌ��Ȃ̂ŔO�̂���64bit�̐��l�Q�������Ă����B
			output_file_name = output_file_name + "_" + to_hex(r.rand<u64>()) + to_hex(r.rand<u64>());
		}

		std::cout << "gensfen2018 : " << endl
			<< "  search_depth = " << search_depth << " to " << search_depth2 << endl
			<< "  loop_max = " << loop_max << endl
			<< "  eval_limit = " << eval_limit << endl
			<< "  thread_num (set by USI setoption) = " << thread_num << endl
			<< "  book_moves (set by USI setoption) = " << Options["BookMoves"] << endl
			<< "  write_minply            = " << write_minply << endl
			<< "  write_maxply            = " << write_maxply << endl
			<< "  output_file_name        = " << output_file_name << endl
			<< "  use_eval_hash           = " << use_eval_hash << endl
			<< "  save_every              = " << save_every << endl
			<< "  random_file_name        = " << random_file_name << endl
			<< "  node_info_hash_size[MB] = " << node_info_hash_size;

		// Options["Threads"]�̐������X���b�h������Ď��s�B
		{
			SfenWriter sw(output_file_name, thread_num);
			sw.save_every = save_every;

			MultiThinkGenSfen2018 multi_think(search_depth, search_depth2, sw , node_info_hash_size);
			multi_think.set_loop_max(loop_max);
			multi_think.eval_limit = eval_limit;
			multi_think.write_minply = write_minply;
			multi_think.write_maxply = write_maxply;
			multi_think.start_file_write_worker();
			multi_think.go_think();

			// SfenWriter�̃f�X�g���N�^��join����̂ŁAjoin���I����Ă���I�������Ƃ������b�Z�[�W��
			// �\��������ׂ��Ȃ̂ł������u���b�N�ň͂ށB
		}

		std::cout << "gensfen2018 finished." << endl;

#if defined(USE_GLOBAL_OPTIONS)
		// GlobalOptions�̕����B
		GlobalOptions = oldGlobalOptions;
#endif

	}
}


#endif // defined(EVAL_LEARN) && defined(USE_GENSFEN2018)

