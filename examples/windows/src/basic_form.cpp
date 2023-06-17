﻿#include "basic_form.h"

#include "stdafx.h"
#include "ui_wnd.h"
#include "main.h"

const std::wstring BasicForm::kClassName = L"Basic";

BasicForm::BasicForm(MainThread *ui_thread) :
        ui_thread_(ui_thread) {
}

BasicForm::~BasicForm() {
}

std::wstring BasicForm::GetSkinFolder() {
    return L"basic";
}

std::wstring BasicForm::GetSkinFile() {
    return L"basic.xml";
}

std::wstring BasicForm::GetWindowClassName() const {
    return kClassName;
}

//
void BasicForm::InitWindow() {
    btn_device_start_ = dynamic_cast<ui::Button *>(FindControl(L"btn_device_start"));
    btn_prev_ = dynamic_cast<ui::Button *>(FindControl(L"btn_prev"));
    btn_push_ = dynamic_cast<ui::Button *>(FindControl(L"btn_push"));
    edit_host_ = dynamic_cast<ui::RichEdit *>(FindControl(L"edit_xrtc_host"));
    edit_uid_ = dynamic_cast<ui::RichEdit *>(FindControl(L"edit_uid"));
    edit_stream_name_ = dynamic_cast<ui::RichEdit *>(FindControl(L"edit_stream_name"));

    xrtc::XRTCEngine::Init(this);

    InitComboCam();
    m_pRoot->AttachBubbledEvent(ui::kEventAll, nbase::Bind(&BasicForm::Notify, this, std::placeholders::_1));

//    uint32_t cnt = xrtc::XRTCEngine::GetCameraCount();

}

LRESULT BasicForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
    PostQuitMessage(0L);
    return __super::OnClose(uMsg, wParam, lParam, bHandled);
}

//
void BasicForm::ShowMax() {
    SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0);
}

ui::Control *BasicForm::CreateControl(const std::wstring &pstrClass) {
    //自定义控件
    if (!_tcscmp(pstrClass.c_str(), _T("VideoWnd"))) {
        CWndUI *wnd = new CWndUI(this->m_hWnd);
        return wnd;
    }

    return nullptr;
}

bool BasicForm::Notify(ui::EventArgs *msg) {
    std::wstring name = msg->pSender->GetName();
    if (msg->Type == ui::kEventClick) {
        if (L"btn_device_start" == name) {
            OnBtnDeviceStartClick();
        } else if (L"btn_prev" == name) {
			OnBtnPreviewClick();
        } else if (L"btn_push" == name) {
			OnBtnPushClick();
        }
    }
    return true;
}

void BasicForm::InitComboCam() {
    int total = xrtc::XRTCEngine::GetCameraCount();
    if (total <= 0) {
        return;
    }

    combo_cam_ = dynamic_cast<ui::Combo *>(FindControl(L"cam_combo"));
    if (!combo_cam_) {
        return;
    }


    for (int i = 0; i < total; ++i) {
        std::string device_name;
        std::string device_id;
        xrtc::XRTCEngine::GetCameraInfo(i, device_name, device_id);
        ui::ListContainerElement *element = new ui::ListContainerElement();
        element->SetClass(L"listitem");
        element->SetFixedHeight(30);
        element->SetText(nbase::UTF8ToUTF16(device_name));
        element->SetDataID(nbase::UTF8ToUTF16(device_id));
        element->SetTextPadding({6, 0, 6, 0});
        combo_cam_->Add(element);
    }

    // 默认选中第一条数据
    int count = combo_cam_->GetCount();
    if (count > 0) {
        combo_cam_->SelectItem(0);
    }
    combo_cam_->AttachSelect(nbase::Bind(&BasicForm::OnComboCamItemSelected, this,
                                         std::placeholders::_1));
}

bool BasicForm::OnComboCamItemSelected(ui::EventArgs *msg) {
    return true;
}

void BasicForm::OnBtnDeviceStartClick() {
    btn_device_start_->SetEnabled(false);
    if(!device_init_){
        if(StartDevice()){
            btn_device_start_->SetText(L"停止音视频设备");
        }
    }else{
        if(StopDevice()){
            btn_device_start_->SetText(L"启动音视频设备");
        }
    }

    btn_device_start_->SetEnabled(true);

}

bool BasicForm::StartDevice() {
	if (!combo_cam_) {
		return false;
	}

	// 获取device_id
	int index = combo_cam_->GetCurSel();
	auto item = combo_cam_->GetItemAt(index);
	std::wstring w_device_id = item->GetDataID();
	cam_source_ = xrtc::XRTCEngine::CreateCamSource(
		nbase::UTF16ToUTF8(w_device_id));
	cam_source_->Start();

    device_init_ = true;
	return true;
}
//
bool BasicForm::StopDevice() {
	if (!device_init_ || !cam_source_) {
		return false;
	}

	cam_source_->Stop();
	cam_source_->Destroy();
	cam_source_ = nullptr;

	device_init_ = false;

	return true;
}

void BasicForm::OnBtnPreviewClick() {
	btn_prev_->SetEnabled(false);

	if (!xrtc_preview_) {
		if (StartPreview()) {
			btn_prev_->SetText(L"停止预览");
		}
	}
	else {
		if (StopPreview()) {
			btn_prev_->SetText(L"本地预览");
		}
	}

	btn_prev_->SetEnabled(true);
}

bool BasicForm::StartPreview() {
	if (!cam_source_) {
		ShowToast(L"预览失败：没有视频源", true);
		return false;
	}

	HWND hwnd = nullptr;
	CWndUI* local_video = dynamic_cast<CWndUI*>(FindControl(L"local"));
	if (local_video) {
		hwnd = local_video->GetVideoWnd();
	}

	if (!hwnd) {
		ShowToast(L"预览失败：没有显示窗口", true);
		return false;
	}

	xrtc_render_ = xrtc::XRTCEngine::CreateRender(hwnd);
	xrtc_preview_ = xrtc::XRTCEngine::CreatePreview(cam_source_, xrtc_render_);
	xrtc_preview_->Start();

	return true;
}

bool BasicForm::StopPreview() {
	if (!xrtc_preview_) {
		return false;
	}

	xrtc_preview_->Stop();
	xrtc_preview_->Destroy();
	xrtc_preview_ = nullptr;

	ShowToast(L"停止本地预览成功", false);

	return true;
}

void BasicForm::OnBtnPushClick() {
	btn_push_->SetEnabled(false);
	if (!xrtc_pusher_) {
		if (StartPush()) {
			btn_push_->SetText(L"停止推流");
		}
	}
	else {
		if (StopPush()) {
			btn_push_->SetText(L"开始推流");
		}
	}

	btn_push_->SetEnabled(true);
}

bool BasicForm::StartPush() {
	if (!cam_source_) {
		ShowToast(L"推流失败：没有视频源", true);
		return false;
	}

	std::string url = "xrtc://" + nbase::UTF16ToUTF8(edit_host_->GetText())
		+ "/push?uid=" + nbase::UTF16ToUTF8(edit_uid_->GetText())
		+ "&streamName=" + nbase::UTF16ToUTF8(edit_stream_name_->GetText());

	xrtc_pusher_ = xrtc::XRTCEngine::CreatePusher(cam_source_);

	xrtc_pusher_->StartPush(url);

	return true;
}

bool BasicForm::StopPush() {
	if (!xrtc_pusher_) {
		return true;
	}

//	xrtc_pusher_->StopPush();
//	xrtc_pusher_->Destroy();
	xrtc_pusher_ = nullptr;

	return true;
}

void BasicForm::ShowToast(const std::wstring &toast, bool err) {
    CallOnUIThread([=]() {
        ui::Label *toast_text = dynamic_cast<ui::Label *>(FindControl(L"toast_text"));
        if (toast_text) {
            if (err) {
                toast_text->SetStateTextColor(ui::kControlStateNormal, L"red");
            }

            toast_text->SetText(toast);
        }
    });
}

void BasicForm::CallOnUIThread(const std::function<void(void)> &task) {
    ui_thread_->message_loop()->PostTask(task);
}

void BasicForm::OnVideoSourceSuccess(xrtc::IVideoSource* video_source) {
//	 api_thread线程回调
	device_init_ = true;
	ShowToast(L"摄像头启动成功", false);
}

void BasicForm::OnVideoSourceFailed(xrtc::IVideoSource* video_source,
	xrtc::XRTCError err)
{
	std::wstring wstr = nbase::StringPrintf(L"摄像头启动设备，err_code: %d", err);
	ShowToast(wstr, true);
}

void BasicForm::OnPreviewSuccess(xrtc::XRTCPreview*) {
	ShowToast(L"本地预览成功", false);
}

void BasicForm::OnPreviewFailed(xrtc::XRTCPreview*, xrtc::XRTCError err) {
	std::wstring msg = nbase::StringPrintf(L"本地预览失败, err: %d", err);
	ShowToast(msg, true);

	if (xrtc_preview_) {
		xrtc_preview_->Stop();
		xrtc_preview_->Destroy();
		xrtc_preview_ = nullptr;
	}
    // 因为这个方法是在api thread中被调用的
	CallOnUIThread([=] {
		btn_prev_->SetText(L"本地预览");
	});
}
//
//void BasicForm::OnPushSuccess(xrtc::XRTCPusher*) {
//	ShowToast(L"xrtc推流成功", false);
//}

//void BasicForm::OnPushFailed(xrtc::XRTCPusher*, xrtc::XRTCError err) {
//	std::wstring err_msg = nbase::StringPrintf(L"xrtc推流失败，err: %d", err);
//	ShowToast(err_msg, true);
//}

//void BasicForm::OnNetworkInfo(int64_t rtt_ms, int32_t packets_lost,
//	uint8_t fraction_lost, uint32_t jitter)
//{
//	std::wstring network_info = nbase::StringPrintf(L"rtt=%lldms 累计丢包数=%d 丢包指数=%u jitter=%u",
//		rtt_ms, packets_lost, fraction_lost, jitter);
//	CallOnUIThread([=]() {
//		if (label_network_tips_) {
//			label_network_tips_->SetText(network_info);
//		}
//	});
//}




//