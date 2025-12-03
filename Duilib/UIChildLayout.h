#pragma once

#ifndef __UICHILDLAYOUT_H__
#define __UICHILDLAYOUT_H__

namespace DuiLib
{
	class CChildLayoutUI : public CContainerUI
	{
	public:
		CChildLayoutUI();

		void Init();
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void SetChildLayoutXML(CDuiString pXML);
		DuiLib::CDuiString GetChildLayoutXML();
		virtual LPVOID GetInterface(LPCTSTR pstrName);
		virtual LPCTSTR GetClass() const;

	private:
		DuiLib::CDuiString m_pstrXMLFile;
	};
} // namespace DuiLib
#endif // __UICHILDLAYOUT_H__
