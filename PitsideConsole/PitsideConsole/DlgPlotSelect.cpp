#include "Stdafx.h"
#include "DlgPlotSelect.h"
#include "resource.h"

#include "PitsideConsole.h"
#include "LapReceiver.h"
#include "ArtSQL/ArtSQLite.h"
#include "LapData.h"
#include "DlgRaceSelect.h"
#include "LapPainter.h"

bool fCancelled = false;
TCHAR szTemp[512];
map<int,CExtendedLap*> m_mapLaps; // maps from iLapId to a lap object
CExtendedLap* m_pReferenceLap;
ArtListBox m_LapList;
vector<CExtendedLap*> GetAllLaps();
int TotalYChannels;

void LoadLaps(ILapReceiver* pReceiver, int m_iRaceId)
  {
    vector<const ILap*> laps = pReceiver->GetLaps(m_iRaceId);
    m_pReferenceLap = NULL;
    for(int x = 0;x < laps.size(); x++)
    {
		const ILap* pLap = laps[x];
		// we don't have this lap yet, so let's put it in
		CExtendedLap* pNewLap = new CExtendedLap(pLap, m_pReferenceLap, pReceiver, true);
		m_mapLaps[pLap->GetLapId()] = pNewLap;
    }
  }

vector<CExtendedLap*> GetAllLaps()
  {
    set<LPARAM> setSelectedLaps = m_LapList.GetSelectedItemsData();
    vector<CExtendedLap*> lstLaps;
    for(map<int,CExtendedLap*>::const_iterator i = m_mapLaps.begin(); i != m_mapLaps.end(); i++)
    {
      CExtendedLap* pLap = i->second;
      lstLaps.push_back(pLap);
    }

    return lstLaps;
  }

    void CPlotSelectDlg::InitPlotPrefs(HWND hWnd, LPARAM lParam)
	{
		//  We need to get all of the Y-axis data channels being displayed
		// First load all laps and get their channels
		vector<RACEDATA> lstRaces = m_pLapDB->GetRaces();
		//	Pull the data channels for each lap from this race instance m_iRaceId
		m_pPlotResults->iPlotId = m_iRaceId;	//	Get the Selected Race ID.
		m_LapList.Clear();	//  Clear list of laps in memory and reload them.
		if (m_pLapDB->GetLapCount(m_pPlotResults->iPlotId) > 1)	//	If race has more than 1 lap, let's get the laps / channels
		{
			LoadLaps(m_pLapDB, m_pPlotResults->iPlotId);	//	Load all of the laps for this Race ID
			GetAllLaps();	//	Load up lstLaps with all available laps
		}
		//	Now that we have the lap list from the DB, let's get the data channels in them

		m_sfYAxis.Clear();
		set<LPARAM> setYSelected;
		vector<const ILap*> lstLaps = m_pLapDB->GetLaps(m_pPlotResults->iPlotId);
		for(int x = 0; x < lstLaps.size(); x++)
		{
			// build up the data channel set
			TotalYChannels = 1;
			TCHAR szDataChannelName[MAX_PATH];
			set<DATA_CHANNEL> channels = m_pLapDB->GetAvailableChannels(lstLaps[x]->GetLapId()); // get all the channels that this lap has
			for(set<DATA_CHANNEL>::const_iterator i = channels.begin(); i != channels.end(); i++) // loop through them, insert them into our "all data channels" set
			{
				setYSelected.insert(*i);
				GetDataChannelName(*i, szDataChannelName, NUMCHARS(szDataChannelName));
				m_sfYAxis.AddString(szDataChannelName,*i);
				wcscpy(m_sfLapOpts->m_PlotPrefs[TotalYChannels].m_ChannelName, szDataChannelName);
				m_sfLapOpts->m_PlotPrefs[TotalYChannels].iDataChannel = *i;	//	Add the DATA_CHANNEL enum into the PP array
				TotalYChannels = TotalYChannels + 1;
			}
		}
		//  Display all of the data channels.
			HWND p_hWnd;
			TCHAR szTemp[512];

			//	Initialize the Plot Prefs dialog box
			for (int z=1; z <= setYSelected.size(); z++)
			{
				wcscpy(szTemp, m_sfLapOpts->m_PlotPrefs[z].m_ChannelName);	//	Load the dialog with Channel names
				p_hWnd = GetDlgItem(hWnd, IDC_PLOTTYPE_CHANNEL0 + z);
				SendMessage(p_hWnd, WM_SETTEXT, 0, (LPARAM)szTemp);

				if (m_sfLapOpts->m_PlotPrefs[z].iPlotView)	//	Load the dialog with the radio buttons
				{
					CheckRadioButton(hWnd, IDC_PLOTTYPE_GRAPH0 + z*2, IDC_PLOTTYPE_VALUE0 + z*2, IDC_PLOTTYPE_GRAPH0 + z*2);
				}
				else 
				{
					CheckRadioButton(hWnd, IDC_PLOTTYPE_GRAPH0 + z*2, IDC_PLOTTYPE_VALUE0 + z*2, IDC_PLOTTYPE_VALUE0 + z*2);
				}

				//	Now let's get the state of the check box and assign the array accordingly
				if (m_sfLapOpts->m_PlotPrefs[z].iTransformYesNo == true)
				{
					Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + z, BST_CHECKED);
				}
				else
				{
					Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + z, BST_UNCHECKED);
				}

				TCHAR szText[MAX_PATH];	//	Load the dialog with the alarm limits
				swprintf (szText, NUMCHARS(szText), L"%9.2f%", m_sfLapOpts->m_PlotPrefs[z].fMinValue);
				SetDlgItemText(hWnd, IDC_PLOTTYPE_LOWLIMIT0 + z, LPCWSTR(&szText));       
				swprintf (szText, NUMCHARS(szText), L"%9.2f%", m_sfLapOpts->m_PlotPrefs[z].fMaxValue);
				SetDlgItemText(hWnd, IDC_PLOTTYPE_HIGHLIMIT0 + z, LPCWSTR(&szText));       
				swprintf (szText, NUMCHARS(szText), L"%5.4f%", m_sfLapOpts->m_PlotPrefs[z].fTransAValue);
				SetDlgItemText(hWnd, IDC_PLOTTYPE_TRANS_A0 + z, LPCWSTR(&szText));       
				swprintf (szText, NUMCHARS(szText), L"%5.4f%", m_sfLapOpts->m_PlotPrefs[z].fTransBValue);
				SetDlgItemText(hWnd, IDC_PLOTTYPE_TRANS_B0 + z, LPCWSTR(&szText));       
				swprintf (szText, NUMCHARS(szText), L"%5.4f%", m_sfLapOpts->m_PlotPrefs[z].fTransCValue);
				SetDlgItemText(hWnd, IDC_PLOTTYPE_TRANS_C0 + z, LPCWSTR(&szText));       
			}
    }

      void CPlotSelectDlg::SetPlotPrefs(HWND hWnd, set<DATA_CHANNEL> setAvailable)
	  {
		   //  Display all of the limits for all data channels

			HWND p_hWnd;
			TCHAR szTemp[512];
			for (int z=1; z <= TotalYChannels - 1; z++)
			{
				wcscpy(szTemp, m_sfLapOpts->m_PlotPrefs[z].m_ChannelName);	//	Load the dialog with Channel names
				p_hWnd = GetDlgItem(hWnd, IDC_PLOTTYPE_CHANNEL0 + z);
				SendMessage(p_hWnd, WM_SETTEXT, 0, (LPARAM)szTemp);

				TCHAR szText[MAX_PATH];	//	Load the dialog with the alarm limits
				swprintf (szText, NUMCHARS(szText), L"%9.2f%", m_sfLapOpts->m_PlotPrefs[z].fMinValue);
				SetDlgItemText(hWnd, IDC_PLOTTYPE_LOWLIMIT0 + z, LPCWSTR(&szText));       
				swprintf (szText, NUMCHARS(szText), L"%9.2f%", m_sfLapOpts->m_PlotPrefs[z].fMaxValue);
				SetDlgItemText(hWnd, IDC_PLOTTYPE_HIGHLIMIT0 + z, LPCWSTR(&szText));       

				swprintf (szText, NUMCHARS(szText), L"%5.4f%", m_sfLapOpts->m_PlotPrefs[z].fTransAValue);
				SetDlgItemText(hWnd, IDC_PLOTTYPE_TRANS_A0 + z, LPCWSTR(&szText));       
				swprintf (szText, NUMCHARS(szText), L"%5.4f%", m_sfLapOpts->m_PlotPrefs[z].fTransBValue);
				SetDlgItemText(hWnd, IDC_PLOTTYPE_TRANS_B0 + z, LPCWSTR(&szText));       
				swprintf (szText, NUMCHARS(szText), L"%5.4f%", m_sfLapOpts->m_PlotPrefs[z].fTransCValue);
				SetDlgItemText(hWnd, IDC_PLOTTYPE_TRANS_C0 + z, LPCWSTR(&szText));       

				if (m_sfLapOpts->m_PlotPrefs[z].iPlotView == true)
				{
					CheckRadioButton(hWnd, IDC_PLOTTYPE_GRAPH0 + z*2, IDC_PLOTTYPE_VALUE0 + z*2, IDC_PLOTTYPE_GRAPH0 + z*2);
				}
				else 
				{
					CheckRadioButton(hWnd, IDC_PLOTTYPE_GRAPH0 + z*2, IDC_PLOTTYPE_VALUE0 + z*2, IDC_PLOTTYPE_VALUE0 + z*2);
				}

				//	Now let's get the state of the check box and assign the array accordingly
				if (m_sfLapOpts->m_PlotPrefs[z].iTransformYesNo == true)
				{
					Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + z, BST_CHECKED);
				}
				else
				{
					Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + z, BST_UNCHECKED);
				}

			}
	  }

    LRESULT CPlotSelectDlg::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
	  switch(uMsg)
      {
        case WM_INITDIALOG:
        {
          //  Initialize all data channels, if not already set by user.
          if (m_sfLapOpts->m_PlotPrefs[1].m_ChannelName[512] == m_sfLapOpts->m_PlotPrefs[0].m_ChannelName[512])
		  {
            InitPlotPrefs(hWnd, lParam);
          }
          else
          {
            set<DATA_CHANNEL> setAvailable;
            SetPlotPrefs(hWnd, setAvailable);
          }
          return TRUE;
        }

        case WM_COMMAND:
        {
          switch(LOWORD(wParam))
          {
			  case IDC_PLOTTYPE_GRAPH1:
			  {
				m_sfLapOpts->m_PlotPrefs[1].iPlotView = true;
				break;
			  }
			  case IDC_PLOTTYPE_VALUE1:
			  {
				m_sfLapOpts->m_PlotPrefs[1].iPlotView = false;
				break;
			  }
			  case IDC_PLOTTYPE_GRAPH2:
			  {
				m_sfLapOpts->m_PlotPrefs[2].iPlotView = true;
				break;
			  }
			  case IDC_PLOTTYPE_VALUE2:
			  {
				m_sfLapOpts->m_PlotPrefs[2].iPlotView = false;
				break;
			  }
			  case IDC_PLOTTYPE_GRAPH3:
			  {
				m_sfLapOpts->m_PlotPrefs[3].iPlotView = true;
				break;
			  }
			  case IDC_PLOTTYPE_VALUE3:
			  {
				m_sfLapOpts->m_PlotPrefs[3].iPlotView = false;
				break;
			  }
			  case IDC_PLOTTYPE_GRAPH4:
			  {
				m_sfLapOpts->m_PlotPrefs[4].iPlotView = true;
				break;
			  }
			  case IDC_PLOTTYPE_VALUE4:
			  {
				m_sfLapOpts->m_PlotPrefs[4].iPlotView = false;
				break;
			  }
			  case IDC_PLOTTYPE_GRAPH5:
			  {
				m_sfLapOpts->m_PlotPrefs[5].iPlotView = true;
				break;
			  }
			  case IDC_PLOTTYPE_VALUE5:
			  {
				m_sfLapOpts->m_PlotPrefs[5].iPlotView = false;
				break;
			  }
			  case IDC_PLOTTYPE_GRAPH6:
			  {
				m_sfLapOpts->m_PlotPrefs[6].iPlotView = true;
				break;
			  }
			  case IDC_PLOTTYPE_VALUE6:
			  {
				m_sfLapOpts->m_PlotPrefs[6].iPlotView = false;
				break;
			  }
			  case IDC_PLOTTYPE_GRAPH7:
			  {
				m_sfLapOpts->m_PlotPrefs[7].iPlotView = true;
				break;
			  }
			  case IDC_PLOTTYPE_VALUE7:
			  {
				m_sfLapOpts->m_PlotPrefs[7].iPlotView = false;
				break;
			  }
			  case IDC_PLOTTYPE_GRAPH8:
			  {
				m_sfLapOpts->m_PlotPrefs[8].iPlotView = true;
				break;
			  }
			  case IDC_PLOTTYPE_VALUE8:
			  {
				m_sfLapOpts->m_PlotPrefs[8].iPlotView = false;
				break;
			  }
			  case IDC_PLOTTYPE_GRAPH9:
			  {
				m_sfLapOpts->m_PlotPrefs[9].iPlotView = true;
				break;
			  }
			  case IDC_PLOTTYPE_VALUE9:
			  {
				m_sfLapOpts->m_PlotPrefs[9].iPlotView = false;
				break;
			  }
			  case IDC_PLOTTYPE_GRAPH10:
			  {
				m_sfLapOpts->m_PlotPrefs[10].iPlotView = true;
				break;
			  }
			  case IDC_PLOTTYPE_VALUE10:
			  {
				m_sfLapOpts->m_PlotPrefs[10].iPlotView = false;
				break;
			  }
			  case IDC_PLOTTYPE_GRAPH11:
			  {
				m_sfLapOpts->m_PlotPrefs[11].iPlotView = true;
				break;
			  }
			  case IDC_PLOTTYPE_VALUE11:
			  {
				m_sfLapOpts->m_PlotPrefs[11].iPlotView = false;
				break;
			  }
			  case IDC_PLOTTYPE_GRAPH12:
			  {
				m_sfLapOpts->m_PlotPrefs[12].iPlotView = true;
				break;
			  }
			  case IDC_PLOTTYPE_VALUE12:
			  {
				m_sfLapOpts->m_PlotPrefs[12].iPlotView = false;
				break;
			  }
			  case IDC_PLOTTYPE_GRAPH13:
			  {
				m_sfLapOpts->m_PlotPrefs[13].iPlotView = true;
				break;
			  }
			  case IDC_PLOTTYPE_VALUE13:
			  {
				m_sfLapOpts->m_PlotPrefs[13].iPlotView = false;
				break;
			  }
			  case IDC_PLOTTYPE_GRAPH14:
			  {
				m_sfLapOpts->m_PlotPrefs[14].iPlotView = true;
				break;
			  }
			  case IDC_PLOTTYPE_VALUE14:
			  {
				m_sfLapOpts->m_PlotPrefs[14].iPlotView = false;
				break;
			  }
			  case IDC_PLOTTYPE_GRAPH15:
			  {
				m_sfLapOpts->m_PlotPrefs[15].iPlotView = true;
				break;
			  }
			  case IDC_PLOTTYPE_VALUE15:
			  {
				m_sfLapOpts->m_PlotPrefs[15].iPlotView = false;
				break;
			  }
			  case IDC_PLOTTYPE_GRAPH16:
			  {
				m_sfLapOpts->m_PlotPrefs[16].iPlotView = true;
				break;
			  }
			  case IDC_PLOTTYPE_VALUE16:
			  {
				m_sfLapOpts->m_PlotPrefs[16].iPlotView = false;
				break;
			  }
			  case IDC_PLOTTYPE_GRAPH17:
			  {
				m_sfLapOpts->m_PlotPrefs[17].iPlotView = true;
				break;
			  }
			  case IDC_PLOTTYPE_VALUE17:
			  {
				m_sfLapOpts->m_PlotPrefs[17].iPlotView = false;
				break;
			  }
			  case IDC_PLOTTYPE_GRAPH18:
			  {
				m_sfLapOpts->m_PlotPrefs[18].iPlotView = true;
				break;
			  }
			  case IDC_PLOTTYPE_VALUE18:
			  {
				m_sfLapOpts->m_PlotPrefs[18].iPlotView = false;
				break;
			  }
			  case IDC_PLOTTYPE_GRAPH19:
			  {
				m_sfLapOpts->m_PlotPrefs[19].iPlotView = true;
				break;
			  }
			  case IDC_PLOTTYPE_VALUE19:
			  {
				m_sfLapOpts->m_PlotPrefs[19].iPlotView = false;
				break;
			  }
			  case IDC_PLOTTYPE_GRAPH20:
			  {
				m_sfLapOpts->m_PlotPrefs[20].iPlotView = true;
				break;
			  }
			  case IDC_PLOTTYPE_VALUE20:
			  {
				m_sfLapOpts->m_PlotPrefs[20].iPlotView = false;
				break;
			  }
			  case IDC_PLOTTYPE_FILTERYESNO1:
			  {
				  if (m_sfLapOpts->m_PlotPrefs[1].iTransformYesNo == true)
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 1, BST_UNCHECKED);
					  m_sfLapOpts->m_PlotPrefs[1].iTransformYesNo = false;
				  }
				  else
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 1, BST_CHECKED);
					  m_sfLapOpts->m_PlotPrefs[1].iTransformYesNo = true;
				  }
				  break;
			  }
			  case IDC_PLOTTYPE_FILTERYESNO2:
			  {
				  if (m_sfLapOpts->m_PlotPrefs[2].iTransformYesNo == true)
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 2, BST_UNCHECKED);
					  m_sfLapOpts->m_PlotPrefs[2].iTransformYesNo = false;
				  }
				  else
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 2, BST_CHECKED);
					  m_sfLapOpts->m_PlotPrefs[2].iTransformYesNo = true;
				  }
				  break;
			  }
			  case IDC_PLOTTYPE_FILTERYESNO3:
			  {
				  if (m_sfLapOpts->m_PlotPrefs[3].iTransformYesNo == true)
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 3, BST_UNCHECKED);
					  m_sfLapOpts->m_PlotPrefs[3].iTransformYesNo = false;
				  }
				  else
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 3, BST_CHECKED);
					  m_sfLapOpts->m_PlotPrefs[3].iTransformYesNo = true;
				  }
				  break;
			  }
			  case IDC_PLOTTYPE_FILTERYESNO4:
			  {
				  if (m_sfLapOpts->m_PlotPrefs[4].iTransformYesNo == true)
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 4, BST_UNCHECKED);
					  m_sfLapOpts->m_PlotPrefs[4].iTransformYesNo = false;
				  }
				  else
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 4, BST_CHECKED);
					  m_sfLapOpts->m_PlotPrefs[4].iTransformYesNo = true;
				  }
				  break;
			  }
			  case IDC_PLOTTYPE_FILTERYESNO5:
			  {
				  if (m_sfLapOpts->m_PlotPrefs[5].iTransformYesNo == true)
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 5, BST_UNCHECKED);
					  m_sfLapOpts->m_PlotPrefs[5].iTransformYesNo = false;
				  }
				  else
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 5, BST_CHECKED);
					  m_sfLapOpts->m_PlotPrefs[5].iTransformYesNo = true;
				  }
				  break;
			  }
			  case IDC_PLOTTYPE_FILTERYESNO6:
			  {
				  if (m_sfLapOpts->m_PlotPrefs[6].iTransformYesNo == true)
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 6, BST_UNCHECKED);
					  m_sfLapOpts->m_PlotPrefs[6].iTransformYesNo = false;
				  }
				  else
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 6, BST_CHECKED);
					  m_sfLapOpts->m_PlotPrefs[6].iTransformYesNo = true;
				  }
				  break;
			  }
			  case IDC_PLOTTYPE_FILTERYESNO7:
			  {
				  if (m_sfLapOpts->m_PlotPrefs[7].iTransformYesNo == true)
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 7, BST_UNCHECKED);
					  m_sfLapOpts->m_PlotPrefs[7].iTransformYesNo = false;
				  }
				  else
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 7, BST_CHECKED);
					  m_sfLapOpts->m_PlotPrefs[7].iTransformYesNo = true;
				  }
				  break;
			  }
			  case IDC_PLOTTYPE_FILTERYESNO8:
			  {
				  if (m_sfLapOpts->m_PlotPrefs[8].iTransformYesNo == true)
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 8, BST_UNCHECKED);
					  m_sfLapOpts->m_PlotPrefs[8].iTransformYesNo = false;
				  }
				  else
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 8, BST_CHECKED);
					  m_sfLapOpts->m_PlotPrefs[8].iTransformYesNo = true;
				  }
				  break;
			  }
			  case IDC_PLOTTYPE_FILTERYESNO9:
			  {
				  if (m_sfLapOpts->m_PlotPrefs[9].iTransformYesNo == true)
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 9, BST_UNCHECKED);
					  m_sfLapOpts->m_PlotPrefs[9].iTransformYesNo = false;
				  }
				  else
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 9, BST_CHECKED);
					  m_sfLapOpts->m_PlotPrefs[9].iTransformYesNo = true;
				  }
				  break;
			  }
			  case IDC_PLOTTYPE_FILTERYESNO10:
			  {
				  if (m_sfLapOpts->m_PlotPrefs[10].iTransformYesNo == true)
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 10, BST_UNCHECKED);
					  m_sfLapOpts->m_PlotPrefs[10].iTransformYesNo = false;
				  }
				  else
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 10, BST_CHECKED);
					  m_sfLapOpts->m_PlotPrefs[10].iTransformYesNo = true;
				  }
				  break;
			  }
			  case IDC_PLOTTYPE_FILTERYESNO11:
			  {
				  if (m_sfLapOpts->m_PlotPrefs[11].iTransformYesNo == true)
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 11, BST_UNCHECKED);
					  m_sfLapOpts->m_PlotPrefs[11].iTransformYesNo = false;
				  }
				  else
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 11, BST_CHECKED);
					  m_sfLapOpts->m_PlotPrefs[11].iTransformYesNo = true;
				  }
				  break;
			  }
			  case IDC_PLOTTYPE_FILTERYESNO12:
			  {
				  if (m_sfLapOpts->m_PlotPrefs[12].iTransformYesNo == true)
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 12, BST_UNCHECKED);
					  m_sfLapOpts->m_PlotPrefs[12].iTransformYesNo = false;
				  }
				  else
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 12, BST_CHECKED);
					  m_sfLapOpts->m_PlotPrefs[12].iTransformYesNo = true;
				  }
				  break;
			  }
			  case IDC_PLOTTYPE_FILTERYESNO13:
			  {
				  if (m_sfLapOpts->m_PlotPrefs[13].iTransformYesNo == true)
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 13, BST_UNCHECKED);
					  m_sfLapOpts->m_PlotPrefs[13].iTransformYesNo = false;
				  }
				  else
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 13, BST_CHECKED);
					  m_sfLapOpts->m_PlotPrefs[13].iTransformYesNo = true;
				  }
				  break;
			  }
			  case IDC_PLOTTYPE_FILTERYESNO14:
			  {
				  if (m_sfLapOpts->m_PlotPrefs[14].iTransformYesNo == true)
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 14, BST_UNCHECKED);
					  m_sfLapOpts->m_PlotPrefs[14].iTransformYesNo = false;
				  }
				  else
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 14, BST_CHECKED);
					  m_sfLapOpts->m_PlotPrefs[14].iTransformYesNo = true;
				  }
				  break;
			  }
			  case IDC_PLOTTYPE_FILTERYESNO15:
			  {
				  if (m_sfLapOpts->m_PlotPrefs[15].iTransformYesNo == true)
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 15, BST_UNCHECKED);
					  m_sfLapOpts->m_PlotPrefs[15].iTransformYesNo = false;
				  }
				  else
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 15, BST_CHECKED);
					  m_sfLapOpts->m_PlotPrefs[15].iTransformYesNo = true;
				  }
				  break;
			  }
			  case IDC_PLOTTYPE_FILTERYESNO16:
			  {
				  if (m_sfLapOpts->m_PlotPrefs[16].iTransformYesNo == true)
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 16, BST_UNCHECKED);
					  m_sfLapOpts->m_PlotPrefs[16].iTransformYesNo = false;
				  }
				  else
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 16, BST_CHECKED);
					  m_sfLapOpts->m_PlotPrefs[16].iTransformYesNo = true;
				  }
				  break;
			  }
			  case IDC_PLOTTYPE_FILTERYESNO17:
			  {
				  if (m_sfLapOpts->m_PlotPrefs[17].iTransformYesNo == true)
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 17, BST_UNCHECKED);
					  m_sfLapOpts->m_PlotPrefs[17].iTransformYesNo = false;
				  }
				  else
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 17, BST_CHECKED);
					  m_sfLapOpts->m_PlotPrefs[17].iTransformYesNo = true;
				  }
				  break;
			  }
			  case IDC_PLOTTYPE_FILTERYESNO18:
			  {
				  if (m_sfLapOpts->m_PlotPrefs[18].iTransformYesNo == true)
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 18, BST_UNCHECKED);
					  m_sfLapOpts->m_PlotPrefs[18].iTransformYesNo = false;
				  }
				  else
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 18, BST_CHECKED);
					  m_sfLapOpts->m_PlotPrefs[18].iTransformYesNo = true;
				  }
				  break;
			  }
			  case IDC_PLOTTYPE_FILTERYESNO19:
			  {
				  if (m_sfLapOpts->m_PlotPrefs[19].iTransformYesNo == true)
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 19, BST_UNCHECKED);
					  m_sfLapOpts->m_PlotPrefs[19].iTransformYesNo = false;
				  }
				  else
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 19, BST_CHECKED);
					  m_sfLapOpts->m_PlotPrefs[19].iTransformYesNo = true;
				  }
				  break;
			  }
			  case IDC_PLOTTYPE_FILTERYESNO20:
			  {
				  if (m_sfLapOpts->m_PlotPrefs[20].iTransformYesNo == true)
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 20, BST_UNCHECKED);
					  m_sfLapOpts->m_PlotPrefs[20].iTransformYesNo = false;
				  }
				  else
				  {
					  Checkbox(hWnd, IDC_PLOTTYPE_FILTERYESNO0 + 20, BST_CHECKED);
					  m_sfLapOpts->m_PlotPrefs[20].iTransformYesNo = true;
				  }
				  break;
			  }
			  case IDC_PLOTTYPE_RESCAN:
			  {
				  //  Initialize all data channels, if not already set by user.
				  //  Assumes first data channel (LONG X) will always be a graph
				  for (int i=0; i < 50; i++)
				  {
						swprintf(m_sfLapOpts->m_PlotPrefs[i].m_ChannelName, L"Velocity");
						m_sfLapOpts->m_PlotPrefs[i].iDataChannel = DATA_CHANNEL_VELOCITY;
						m_sfLapOpts->m_PlotPrefs[i].iPlotView = true;  //  Default to dsplay as a graph
						m_sfLapOpts->m_PlotPrefs[i].fMinValue = -3.0;    //  Set all lower limits to -3.0
						m_sfLapOpts->m_PlotPrefs[i].fMaxValue = 1000000.0;  //  Set all upper limits to 1000000.0
						m_sfLapOpts->m_PlotPrefs[i].iTransformYesNo = false;  //  Default to display as a graph
						m_sfLapOpts->m_PlotPrefs[i].fTransAValue = 0.0;  //  Set all A constants to 0.0
						m_sfLapOpts->m_PlotPrefs[i].fTransBValue = 1.0;  //  Set all B constants to 1.0
						m_sfLapOpts->m_PlotPrefs[i].fTransCValue = 0.0;  //  Set all C constants to 0.0
				  }
				  InitPlotPrefs(hWnd, lParam);
				  return TRUE;
			  }
			  case IDOK:
			  {
				//  Let's get the values for each channel and store it for program execution
				TCHAR szText[MAX_PATH];
				int len;
				float flValue;

				for (int z=1; z <= TotalYChannels -1; z++)
				{
					len = GetWindowTextLength(GetDlgItem(hWnd, IDC_PLOTTYPE_LOWLIMIT0 + z));
					GetDlgItemText(hWnd, IDC_PLOTTYPE_LOWLIMIT0 + z, szText, len+1);
					flValue = _wtof(szText);
					m_sfLapOpts->m_PlotPrefs[z].fMinValue = flValue;
					len = GetWindowTextLength(GetDlgItem(hWnd, IDC_PLOTTYPE_HIGHLIMIT0 + z));
					GetDlgItemText(hWnd, IDC_PLOTTYPE_HIGHLIMIT0 + z, szText, len+1);
					flValue = _wtof(szText);
					m_sfLapOpts->m_PlotPrefs[z].fMaxValue = flValue;

					GetDlgItemText(hWnd, IDC_PLOTTYPE_TRANS_A0 + z, szText, len+1);
					flValue = _wtof(szText);
					m_sfLapOpts->m_PlotPrefs[z].fTransAValue = flValue;
					GetDlgItemText(hWnd, IDC_PLOTTYPE_TRANS_B0 + z, szText, len+1);
					flValue = _wtof(szText);
					m_sfLapOpts->m_PlotPrefs[z].fTransBValue = flValue;
					GetDlgItemText(hWnd, IDC_PLOTTYPE_TRANS_C0 + z, szText, len+1);
					flValue = _wtof(szText);
					m_sfLapOpts->m_PlotPrefs[z].fTransCValue = flValue;
				}
						
				m_pPlotResults->fCancelled = false;
				EndDialog(hWnd,0);
				return TRUE;
			  }
			  case IDCANCEL:
			  {
				m_pPlotResults->fCancelled = true;
				EndDialog(hWnd,0);
				return TRUE;
			  }
			  default:
			  {
				  break;
			  }
		  }	//	End LOWORD loop
		  return TRUE;
        } // end WM_COMMAND
        case WM_CLOSE:
        {
            m_pPlotResults->fCancelled = true;
			EndDialog(hWnd,0);
		    return TRUE;
        }
		default:
	    return FALSE;
      }
    }
//	Function assigns the check box the proper state for that resource
void CPlotSelectDlg::Checkbox(HWND hWnd, UINT DLG_ITEM, UINT CHECKBOX_STATE)
{
		HWND hWnd_CheckBox = GetDlgItem(hWnd, DLG_ITEM);
		CheckDlgButton(hWnd, DLG_ITEM, CHECKBOX_STATE);
}