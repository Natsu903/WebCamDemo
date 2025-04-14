#pragma once
#include <mutex>

struct AVFrame;

void MSleep(unsigned int ms);

class Video_View
{
public:
	enum Format
	{
		RGBA = 0,
		ARGB,
		YUV420P
	};
	enum RenderType
	{
		SDL = 0
	};

	static Video_View* Create(RenderType type=SDL);

	/**
	 * ��ʼ����Ⱦ���� �̰߳�ȫ.
	 * 
	 * \param w ���ڿ��
	 * \param h ���ڸ߶�
	 * \param fmt ���Ƶ����ظ�ʽ��Ĭ��ΪRGBA
	 * \param win_id ���ھ����û�о����´���һ��
	 * \return �Ƿ񴴽��ɹ�
	 */
	virtual bool Init(int w, int h, Format fmt = RGBA, void* win_id = nullptr)=0;

	/**
	 * ��Ⱦͼ�� �̰߳�ȫ.
	 * 
	 * \param data ��Ⱦ�Ķ�����ͼ��
	 * \param linesize һ��������ֽ�������linesize<=0���Զ������С
	 * \return ��Ⱦ�Ƿ�ɹ�
	 */
	virtual bool Draw(const unsigned char* data, int linesize = 0)=0;
	virtual bool Draw(
		const unsigned char* y,int y_pitch,
		const unsigned char* u, int u_pitch, 
		const unsigned char* v, int v_pitch)=0;

	/**
	 * ���ƽӿڷ�װ����Բ�ͬ��������ʹ����ͬ��DrawFrame���ڲ����ò�ͬ��Draw����.
	 * 
	 * \param frame
	 * \return 
	 */
	virtual bool DrawFrame(AVFrame* frame);

	int render_fps()
	{
		return render_fps_;
	}
	
	/**
	 * �����������Դ���رմ���.
	 * 
	 */
	virtual void Close() = 0;

	/**
	 * �������˳��¼�.
	 * 
	 * \return 
	 */
	virtual bool IsExit() = 0;

	//��������
	void Scale(int w, int h)
	{
		scale_w_ = w;
		scale_h_ = h;
	}

protected:
	int render_fps_ = 0;
	int width_=0;		//���ڿ�
	int height_=0;		//���ڸ�
	Format format_=RGBA;//���ظ�ʽ
	std::mutex mtx_;
	int scale_w_ = 0;
	int scale_h_ = 0;
	long long beg_ms_ = 0;
	int count_ = 0;
};

