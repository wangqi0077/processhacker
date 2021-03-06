/*
 * Process Hacker -
 *   PE viewer
 *
 * Copyright (C) 2017 dmex
 *
 * This file is part of Process Hacker.
 *
 * Process Hacker is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Process Hacker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Process Hacker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <peview.h>

VOID PvpProcessElfExports(
    _In_ HWND ListViewHandle
    )
{
    PPH_LIST exports;
    ULONG count = 0;

    PhGetMappedWslImageSymbols(&PvMappedImage, &exports);

    for (ULONG i = 0; i < exports->Count; i++)
    {
        PPH_ELF_IMAGE_SYMBOL_ENTRY export = exports->Items[i];
        INT lvItemIndex;
        WCHAR number[PH_INT32_STR_LEN_1];
        WCHAR pointer[PH_PTR_STR_LEN_1];

        if (!export->ExportSymbol)
            continue;

        PhPrintUInt32(number, ++count);
        lvItemIndex = PhAddListViewItem(ListViewHandle, MAXINT, number, NULL);

        PhPrintPointer(pointer, (PVOID)export->Address);

        PhSetListViewSubItem(ListViewHandle, lvItemIndex, 1, pointer);
        PhSetListViewSubItem(ListViewHandle, lvItemIndex, 2, export->Name);
        PhSetListViewSubItem(ListViewHandle, lvItemIndex, 3, PhaFormatSize(export->Size, -1)->Buffer);
        PhSetListViewSubItem(ListViewHandle, lvItemIndex, 4, PvpGetSymbolTypeName(export->TypeInfo));
        PhSetListViewSubItem(ListViewHandle, lvItemIndex, 5, PvpGetSymbolBindingName(export->TypeInfo));
    }

    PhFreeMappedWslImageSymbols(exports);
}

INT_PTR CALLBACK PvpExlfExportsDlgProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
    )
{
    LPPROPSHEETPAGE propSheetPage;
    PPV_PROPPAGECONTEXT propPageContext;

    if (!PvPropPageDlgProcHeader(hwndDlg, uMsg, lParam, &propSheetPage, &propPageContext))
        return FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            HWND lvHandle;

            lvHandle = GetDlgItem(hwndDlg, IDC_LIST);
            PhSetListViewStyle(lvHandle, TRUE, TRUE);
            PhSetControlTheme(lvHandle, L"explorer");
            PhAddListViewColumn(lvHandle, 0, 0, 0, LVCFMT_LEFT, 40, L"#");
            PhAddListViewColumn(lvHandle, 1, 1, 1, LVCFMT_RIGHT, 80, L"RVA");
            PhAddListViewColumn(lvHandle, 2, 2, 2, LVCFMT_LEFT, 250, L"Name");
            PhAddListViewColumn(lvHandle, 3, 3, 3, LVCFMT_LEFT, 250, L"Size");
            PhAddListViewColumn(lvHandle, 4, 4, 4, LVCFMT_LEFT, 80, L"Type");
            PhAddListViewColumn(lvHandle, 5, 5, 5, LVCFMT_LEFT, 80, L"Binding");
            PhSetExtendedListView(lvHandle);
            PhLoadListViewColumnsFromSetting(L"ImageExportsListViewColumns", lvHandle);

            PvpProcessElfExports(lvHandle);
            ExtendedListView_SortItems(lvHandle);
            
            EnableThemeDialogTexture(hwndDlg, ETDT_ENABLETAB);
        }
        break;
    case WM_DESTROY:
        {
            PhSaveListViewColumnsToSetting(L"ImageExportsListViewColumns", GetDlgItem(hwndDlg, IDC_LIST));
        }
        break;
    case WM_SHOWWINDOW:
        {
            if (!propPageContext->LayoutInitialized)
            {
                PPH_LAYOUT_ITEM dialogItem;

                dialogItem = PvAddPropPageLayoutItem(hwndDlg, hwndDlg,
                    PH_PROP_PAGE_TAB_CONTROL_PARENT, PH_ANCHOR_ALL);
                PvAddPropPageLayoutItem(hwndDlg, GetDlgItem(hwndDlg, IDC_LIST),
                    dialogItem, PH_ANCHOR_ALL);

                PvDoPropPageLayout(hwndDlg);

                propPageContext->LayoutInitialized = TRUE;
            }
        }
        break;
    case WM_NOTIFY:
        {
            PvHandleListViewNotifyForCopy(lParam, GetDlgItem(hwndDlg, IDC_LIST));
        }
        break;
    }

    return FALSE;
}
