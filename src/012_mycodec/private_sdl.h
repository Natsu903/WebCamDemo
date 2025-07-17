#pragma once
#include "video_view.h"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

class PrivateSDL :public Video_View
{
public:
	/**
	 * ��ʼ����Ⱦ���� �̰߳�ȫ.
	 *
	 * \param w ���ڿ��
	 * \param h ���ڸ߶�
	 * \param fmt ���Ƶ����ظ�ʽ��Ĭ��ΪRGBA
	 * \param win_id ���ھ����û�о����´���һ��
	 * \return �Ƿ񴴽��ɹ�
	 */
    bool Init(int w, int h, Format fmt = RGBA, void* win_id = nullptr) override;

	/**
	 * ��Ⱦͼ�� �̰߳�ȫ.
	 *
	 * \param data ��Ⱦ�Ķ�����ͼ��
	 * \param linesize һ��������ֽ�������linesize<=0���Զ������С
	 * \return ��Ⱦ�Ƿ�ɹ�
	 */
    bool Draw(const unsigned char* data, int linesize = 0) override;
	virtual bool Draw(
		const unsigned char* y, int y_pitch,
		const unsigned char* u, int u_pitch,
		const unsigned char* v, int v_pitch);


	/**
	 * �����������Դ���رմ���.
	 *
	 */
	void Close()override;

	/**
	 * �������˳��¼�.
	 *
	 * \return
	 */ 
	bool IsExit()override;

private:
	SDL_Window* win_ = nullptr;
	SDL_Renderer* render_ = nullptr;
	SDL_Texture* texture_ = nullptr;
};

