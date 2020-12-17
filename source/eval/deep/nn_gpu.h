#ifndef __NN_GPU_H_INCLUDED__
#define __NN_GPU_H_INCLUDED__
#include "../../config.h"

#if defined(YANEURAOU_ENGINE_DEEP)

#if defined(ONNXRUNTIME)

	// ONNX Runtime���g���ꍇ�B
	// "docs/���.txt"���m�F���邱�ƁB

	#include <onnxruntime_cxx_api.h>
#endif

#include "nn_evaluate.h"

namespace Eval::dlshogi
{
	// �j���[�����l�b�g�̊��N���X
	class NN
	{
	public:
		// forward�ɓn���������̊m��
		// size������DType���m�ۂ��ĕԂ��Ă����B
		// TODO : �h���N���X����override���邩���B
		void* alloc(size_t size);
		
		// alloc()�Ŋm�ۂ����������̊J��
		// TODO : �h���N���X����override���邩���B
		void free(void* ptr);

		// NN�ɂ�鐄�_�B
		virtual void forward(const int batch_size, NN_Input1* x1, NN_Input2* x2, NN_Output_Policy* y1, NN_Output_Value* y2) = 0;

		// ���f���t�@�C���̓ǂݍ��݁B
		virtual Tools::Result load(const std::string& model_path , int gpu_id) = 0;

		// ���f���t�@�C������n���Ƃ���ɉ�����NN�h���N���X��build���ĕԂ��Ă����B�f�U�p�^�Ō����Ƃ����builder�B
		static std::shared_ptr<NN> build_nn(const std::string& model_path , int gpu_id);

		// �h���N���X���̃f�X�g���N�^�Ăяo����Ăق����̂ł���p�ӂ��Ƃ��B
		virtual ~NN() {}
	};

#if defined(ONNXRUNTIME)

	// ONNXRUNTIME�p
	class NNOnnxRuntime : public NN
	{
	public:
		// ���f���t�@�C���̓ǂݍ��݁B
		virtual Tools::Result load(const std::string& model_path , int gpu_id);

		// NN�ɂ�鐄�_
		virtual void forward(const int batch_size, NN_Input1* x1, NN_Input2* x2, NN_Output_Policy* y1, NN_Output_Value* y2);

	private:
		Ort::Env env;
		std::unique_ptr<Ort::Session> session;
		Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

	};
#endif

} // namespace Eval::dlshogi

#endif // defined(YANEURAOU_ENGINE_DEEP)
#endif // ndef __NN_GPU_H_INCLUDED__
