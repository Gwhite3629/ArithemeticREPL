#ifndef _REPLUTILS_H_
#define _REPLUTILS_H_

#include <stdio.h>
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include "utils.h"
#include "../MathREPL/winhelp.h"
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

//typedef enum COMMANDS {};

void print_screen(map_t* map, char* line);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void MainWindow::CalculateLayout()
{
    if (pRenderTarget != NULL)
    {
        D2D1_SIZE_F size = pRenderTarget->GetSize();
        const float x = size.width / 2;
        const float y = size.height / 2;
        const float radius = min(x, y);
        ellipse = D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius);
    }
}

HRESULT MainWindow::CreateGraphicsResources()
{
    HRESULT hr = S_OK;
    if (pRenderTarget == NULL)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        hr = pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &pRenderTarget);

        hr = pDWriteFactory->CreateTextFormat(
            L"Liberation Mono",
            NULL,
            DWRITE_FONT_WEIGHT_REGULAR,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            48.0f,
            L"en-us",
            &pTextFormat
        );

        hr = pDWriteFactory->CreateTextFormat(
            L"Liberation Mono",
            NULL,
            DWRITE_FONT_WEIGHT_REGULAR,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            48.0f,
            L"en-us",
            &pTextFormatW
        );

        hr = pDWriteFactory->CreateTextFormat(
            L"Liberation Mono",
            NULL,
            DWRITE_FONT_WEIGHT_REGULAR,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            48.0f,
            L"en-us",
            &pTextFormatM
        );

        hr = pDWriteFactory->CreateTextFormat(
            L"Liberation Mono",
            NULL,
            DWRITE_FONT_WEIGHT_REGULAR,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            48.0f,
            L"en-us",
            &pTextFormatH
        );

        hr = pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

        hr = pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

        hr = pTextFormatW->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);

        hr = pTextFormatW->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

        hr = pTextFormatM->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);

        hr = pTextFormatM->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);

        hr = pTextFormatH->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

        hr = pTextFormatH->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);

        if (SUCCEEDED(hr))
        {
            const D2D1_COLOR_F color = D2D1::ColorF(1.0f, 1.0f, 1.0f);
            hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);

            if (SUCCEEDED(hr))
            {
                CalculateLayout();
            }
        }
    }
    return hr;
}

void MainWindow::DiscardGraphicsResources()
{
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBrush);
    SafeRelease(&pTextFormat);
}

void MainWindow::OnPaint()
{
    HRESULT hr = CreateGraphicsResources();
    if (SUCCEEDED(hr))
    {
        PAINTSTRUCT ps;
        BeginPaint(m_hwnd, &ps);
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_RECT_F layoutRect = D2D1::RectF(
            static_cast<FLOAT>(rc.left),
            static_cast<FLOAT>(rc.top),
            static_cast<FLOAT>(rc.right - rc.left),
            static_cast<FLOAT>(rc.bottom - rc.top)
        );
        
        // Get userinput string
        WCHAR* str;
        int nchars = MultiByteToWideChar(CP_ACP, 0, state->userin, -1, NULL, 0);

        str = new WCHAR[nchars];
        
        MultiByteToWideChar(CP_ACP, 0, state->userin, -1, (LPWSTR)str, nchars);

        pRenderTarget->BeginDraw();

        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
        //pRenderTarget->FillEllipse(ellipse, pBrush);
        pRenderTarget->DrawText(str, nchars, pTextFormat, &layoutRect, pBrush);

        if (state->hmax > 1) {
            {
                get_hStr(state);
                WCHAR* str;
                int nchars = MultiByteToWideChar(CP_ACP, 0, state->hStr, -1, NULL, 0);

                str = new WCHAR[nchars];

                MultiByteToWideChar(CP_ACP, 0, state->hStr, -1, (LPWSTR)str, nchars);

                pRenderTarget->DrawText(str, nchars, pTextFormatW, &layoutRect, pBrush);
            }
        }

        if (state->map.size) {
            {
                get_mStr(state);
                WCHAR* str;
                int nchars = MultiByteToWideChar(CP_ACP, 0, state->mStr, -1, NULL, 0);

                str = new WCHAR[nchars];

                MultiByteToWideChar(CP_ACP, 0, state->mStr, -1, (LPWSTR)str, nchars);

                pRenderTarget->DrawText(str, nchars, pTextFormatM, &layoutRect, pBrush);
            }
        }

        {
            get_cursor(state);

            WCHAR* str;
            int nchars = MultiByteToWideChar(CP_ACP, 0, state->cursor, -1, NULL, 0);

            str = new WCHAR[nchars];

            MultiByteToWideChar(CP_ACP, 0, state->cursor, -1, (LPWSTR)str, nchars);

            pRenderTarget->DrawText(str, nchars, pTextFormat, &layoutRect, pBrush);
        }

        pRenderTarget->DrawText(
            L"\
CLEAR: Clear table and history\n\
QUIT: Close program\n\
RUN: Run the file specified"
            , 79, pTextFormatH, &layoutRect, pBrush);

        hr = pRenderTarget->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
        {
            DiscardGraphicsResources();
        }
        EndPaint(m_hwnd, &ps);
    }
}

void MainWindow::Resize()
{
    if (pRenderTarget != NULL)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        pRenderTarget->Resize(size);
        CalculateLayout();
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

#endif