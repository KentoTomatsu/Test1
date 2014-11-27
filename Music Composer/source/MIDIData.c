/******************************************************************************/
/*                                                                            */
/*�@MIDIData.c - MIDIData�\�[�X�t�@�C��                    (C)2002-2009 ����  */
/*                                                                            */
/******************************************************************************/

/* ���̃��W���[���͕��ʂ̂b����ŏ�����Ă���B */
/* ���̃��C�u�����́AGNU �򓙈�ʌ��O���p�����_��(LGPL)�Ɋ�Â��z�z�����B */
/* �v���W�F�N�g�z�[���y�[�W�F"http://openmidiproject.sourceforge.jp/index.html" */
/* MIDI�C�x���g�̎擾�E�ݒ�E�����E�}���E�폜 */
/* MIDI�g���b�N�̎擾�E�ݒ�E�����E�}���E�폜 */
/* MIDI�f�[�^�̐����E�폜�ESMF�t�@�C��(*.mid)���o�� */

/* This library is free software; you can redistribute it and/or */
/* modify it under the terms of the GNU Lesser General Public */
/* License as published by the Free Software Foundation; either */
/* version 2.1 of the License, or (at your option) any later version. */

/* This library is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU */
/* Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public */
/* License along with this library; if not, write to the Free Software */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include "MIDIData.h"

/* �O�̂���verify�}�N������` */
#ifndef verify
#ifdef _DEBUG
#define verify(f) assert(f)
#else
#define verify(f) ((void)(f))
#endif
#endif

/* �ėp�}�N��(�ŏ��A�ő�A���ݍ���) */
#ifndef MIN
#define MIN(A,B) ((A)>(B)?(B):(A))
#endif
#ifndef MAX
#define MAX(A,B) ((A)>(B)?(A):(B))
#endif
#ifndef CLIP
#define CLIP(A,B,C) ((A)>(B)?(A):((B)>(C)?(C):(B)))
#endif

/* 64�r�b�g�̐����^int64_t��` */
/* Microsoft C����Borland C�̏ꍇ�͎��̍s��L���ɂ��Ă��������B */
#if defined(_MSC_VER) || defined(__BORLANDC__) || defined (__WATCOMC__)
typedef __int64 int64_t;
/* GNU����ANSI C99�Ή��R���p�C���̏ꍇ�͎��̍s��L���ɂ��Ă��������B */
#elif (defined(__GNUC__) && \
      (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)))
typedef long long int64_t;
/* �C���e���R���p�C���̏ꍇ�͎��̍s��L���ɂ��Ă��������B*/
#elif defined(__INTEL_COMPILER)
typedef long long int64_t;
/* 64bit�R���p�C���̏ꍇ�͎��̍s��L���ɂ��Ă��������B */
#elif LONG_MAX > 2147483647l
typedef long int64_t;
#endif

/* �ėp�֐� */

/* �w�肵���o�C�g�z��̘a�����߂� */
long Sum (unsigned char* pData, int nLen) {
	long lRet = 0;
	long i;
	assert (pData);
	for (i = 0; i < nLen; i++) {
		lRet += pData[i];
	}
	return lRet;
}

/******************************************************************************/
/*                                                                            */
/*�@MIDIEvent�N���X�֐�                                                       */
/*                                                                            */
/******************************************************************************/

/* ���̓�����ނ̃C�x���g��T�� */
/* (���̊֐��͓����B������Ă��܂�) */
MIDIEvent* __stdcall MIDIEvent_SearchNextSameKindEvent (MIDIEvent* pEvent) {
	MIDIEvent* pSameKindEvent = pEvent->m_pNextEvent;
	while (pSameKindEvent) {
		if (pEvent->m_lKind == pSameKindEvent->m_lKind) {
			break;
		}
		pSameKindEvent = pSameKindEvent->m_pNextEvent;
	}
	return pSameKindEvent;
}

/* �O�̓�����ނ̃C�x���g��T�� */
/* (���̊֐��͓����B������Ă��܂�) */
MIDIEvent* __stdcall MIDIEvent_SearchPrevSameKindEvent (MIDIEvent* pEvent) {
	MIDIEvent* pSameKindEvent = pEvent->m_pPrevEvent;
	while (pSameKindEvent) {
		if (pEvent->m_lKind == pSameKindEvent->m_lKind) {
			break;
		}
		pSameKindEvent = pSameKindEvent->m_pPrevEvent;
	}
	return pSameKindEvent;
}

/* �����C�x���g�̍ŏ��̃C�x���g��Ԃ��B */
/* �����C�x���g�łȂ��ꍇ�ApEvent���g��Ԃ��B*/
MIDIEvent* __stdcall MIDIEvent_GetFirstCombinedEvent (MIDIEvent* pEvent) {
	MIDIEvent* pTempEvent = pEvent;
	assert (pEvent);
	while (pTempEvent->m_pPrevCombinedEvent) {
		pTempEvent = pTempEvent->m_pPrevCombinedEvent;
	}
	return pTempEvent;
}

/* �����C�x���g�̍Ō�̃C�x���g��Ԃ��B */
/* �����C�x���g�łȂ��ꍇ�ApEvent���g��Ԃ��B*/
MIDIEvent* __stdcall MIDIEvent_GetLastCombinedEvent (MIDIEvent* pEvent) {
	MIDIEvent* pTempEvent = pEvent;
	assert (pEvent);
	while (pTempEvent->m_pNextCombinedEvent) {
		pTempEvent = pTempEvent->m_pNextCombinedEvent;
	}
	return pTempEvent;
}


/* �C�x���g���ꎞ�I�ɕ��V������ */
/* (���̊֐��͓����B������Ă��܂�) */
long __stdcall MIDIEvent_SetFloating (MIDIEvent* pEvent) {
	/* �������A�����C�x���g�̉����͍s��Ȃ����Ƃɗv���� */
	assert (pEvent);
	/* �O��̃C�x���g�̃|�C���^�̂Ȃ��ւ� */
	if (pEvent->m_pPrevEvent) {
		pEvent->m_pPrevEvent->m_pNextEvent = pEvent->m_pNextEvent;
	}
	else if (pEvent->m_pParent) {
		((MIDITrack*)(pEvent->m_pParent))->m_pFirstEvent = pEvent->m_pNextEvent;
	}
	if (pEvent->m_pNextEvent) {
		pEvent->m_pNextEvent->m_pPrevEvent = pEvent->m_pPrevEvent;
	}
	else if (pEvent->m_pParent) {
		((MIDITrack*)(pEvent->m_pParent))->m_pLastEvent = pEvent->m_pPrevEvent;
	}
	/* �O��̓���C�x���g�̃|�C���^�̂Ȃ��ւ� */
	if (pEvent->m_pNextSameKindEvent) {
		pEvent->m_pNextSameKindEvent->m_pPrevSameKindEvent = pEvent->m_pPrevSameKindEvent;
	}
	if (pEvent->m_pPrevSameKindEvent) {
		pEvent->m_pPrevSameKindEvent->m_pNextSameKindEvent = pEvent->m_pNextSameKindEvent;
	}
	/* �O��|�C���^��NULL�� */
	pEvent->m_pNextEvent = NULL;
	pEvent->m_pPrevEvent = NULL;
	/* �O��̓���C�x���g�|�C���^NULL�� */
	pEvent->m_pNextSameKindEvent = NULL;
	pEvent->m_pPrevSameKindEvent = NULL;
	/* �e�g���b�N�̃C�x���g����1���炷�B */
	if (pEvent->m_pParent) {
		((MIDITrack*)(pEvent->m_pParent))->m_lNumEvent --;
	}
	pEvent->m_pParent = NULL;
	return 1;
}


/* �O�̃C�x���g��ݒ肷�� */
/* (���̊֐��͓����B������Ă��܂�) */
long __stdcall MIDIEvent_SetPrevEvent (MIDIEvent* pEvent, MIDIEvent* pInsertEvent) {
	assert (pEvent);
	assert (pInsertEvent);
	assert (pEvent != pInsertEvent);
	/* pInsertEvent�����ɂǂ����̃g���b�N�ɑ����Ă���ꍇ�A�ُ�I�� */
	if (!MIDIEvent_IsFloating (pInsertEvent)) {
		return 0;
	}
	/* EOT�̓�d�}���h�~�@�\(�p�~�E��w�̊֐��ŏ���) */
	/*if (MIDIEvent_IsEndofTrack (pInsertEvent)) {
		return 0;
	}*/
	/* EOT�C�x���g�̑O�ɑ}������ꍇ�AEOT�C�x���g�̎�����␳���� */
	if (MIDIEvent_IsEndofTrack (pEvent) && pEvent->m_pNextEvent == NULL) {
		assert (pEvent->m_pNextEvent == NULL);
		if (pEvent->m_lTime < pInsertEvent->m_lTime) { /* 20080622�ǉ� */
			pEvent->m_lTime = pInsertEvent->m_lTime;
		}
	}
	/* �����̐��������Ƃ�Ă��Ȃ��ꍇ�A�����I�ɑ}���C�x���g�̎�����␳���� */
	if (pInsertEvent->m_lTime > pEvent->m_lTime) {
		pInsertEvent->m_lTime = pEvent->m_lTime;
	}
	if (pEvent->m_pPrevEvent) {
		if (pInsertEvent->m_lTime < pEvent->m_pPrevEvent->m_lTime) {
			pInsertEvent->m_lTime = pEvent->m_pPrevEvent->m_lTime;
		}
	}
	/* �O��̃C�x���g�̃|�C���^�̂Ȃ����� */
	pInsertEvent->m_pNextEvent = pEvent;
	pInsertEvent->m_pPrevEvent = pEvent->m_pPrevEvent;
	if (pEvent->m_pPrevEvent) {
		pEvent->m_pPrevEvent->m_pNextEvent = pInsertEvent;
	}
	else if (pEvent->m_pParent) {
		((MIDITrack*)(pEvent->m_pParent))->m_pFirstEvent = pInsertEvent;
	}
	pEvent->m_pPrevEvent = pInsertEvent;
	/* �O��̓���C�x���g�̃|�C���^�̂Ȃ����� */
	if (pInsertEvent->m_pPrevSameKindEvent) {
		pInsertEvent->m_pPrevSameKindEvent->m_pNextSameKindEvent = 
			MIDIEvent_SearchNextSameKindEvent (pInsertEvent->m_pPrevSameKindEvent);
	}
	if (pInsertEvent->m_pNextSameKindEvent) {
		pInsertEvent->m_pNextSameKindEvent->m_pPrevSameKindEvent =
			MIDIEvent_SearchPrevSameKindEvent (pInsertEvent->m_pNextSameKindEvent);
	}
	/* �O��̓���C�x���g�|�C���^�ݒ� */
	pInsertEvent->m_pPrevSameKindEvent = MIDIEvent_SearchPrevSameKindEvent (pInsertEvent);
	if (pInsertEvent->m_pPrevSameKindEvent) {
		pInsertEvent->m_pPrevSameKindEvent->m_pNextSameKindEvent = pInsertEvent;
	}
	pInsertEvent->m_pNextSameKindEvent = MIDIEvent_SearchNextSameKindEvent (pInsertEvent);
	if (pInsertEvent->m_pNextSameKindEvent) {
		pInsertEvent->m_pNextSameKindEvent->m_pPrevSameKindEvent = pInsertEvent;
	}
	/* �e�g���b�N�̃C�x���g����1�������� */
	pInsertEvent->m_pParent = pEvent->m_pParent;
	if (pEvent->m_pParent) {
		((MIDITrack*)(pEvent->m_pParent))->m_lNumEvent ++;
	}
	return 1;
}

/* ���̃C�x���g��ݒ肷�� */
/* (���̊֐��͓����B������Ă��܂�) */
long __stdcall MIDIEvent_SetNextEvent (MIDIEvent* pEvent, MIDIEvent* pInsertEvent) {
	assert (pEvent);
	assert (pInsertEvent);
	assert (pEvent != pInsertEvent);
	/* pInsertEvent�����ɂǂ����̃g���b�N�ɑ����Ă���ꍇ�A�ُ�I�� */
	if (!MIDIEvent_IsFloating (pInsertEvent)) {
		return 0;
	}
	/* EOT�̓�d�}���h�~�@�\(�p�~�E��w�̊֐��ŏ���) */
	/*if (MIDIEvent_IsEndofTrack (pInsertEvent)) {
		return 0;
	}*/
	/* EOT�̌�ɃC�x���g�����悤�Ƃ����ꍇ�AEOT�����Ɉړ����Ȃ��B */
	if (MIDIEvent_IsEndofTrack (pEvent) && pEvent->m_pNextEvent == NULL) {
		assert (0);
		return 0; /* EOT�̎��ɂ̓C�x���g�͗���Ȃ� */
	}
	/* �����̐��������Ƃ�Ă��Ȃ��ꍇ�A�����I�ɑ}���C�x���g�̎�����␳���� */
	if (pInsertEvent->m_lTime < pEvent->m_lTime) {
		pInsertEvent->m_lTime = pEvent->m_lTime;
	}
	if (pEvent->m_pNextEvent) {
		if (pInsertEvent->m_lTime > pEvent->m_pNextEvent->m_lTime) {
			pInsertEvent->m_lTime = pEvent->m_pNextEvent->m_lTime;
		}
	}
	/* �O��̃C�x���g�̃|�C���^�̂Ȃ����� */
	pInsertEvent->m_pNextEvent = pEvent->m_pNextEvent;
	pInsertEvent->m_pPrevEvent = pEvent;
	if (pEvent->m_pNextEvent) {
		pEvent->m_pNextEvent->m_pPrevEvent = pInsertEvent;
	}
	else if (pEvent->m_pParent) { /* �Ō� */
		((MIDITrack*)(pEvent->m_pParent))->m_pLastEvent = pInsertEvent;
	}
	pEvent->m_pNextEvent = pInsertEvent;
	/* �O��̓���C�x���g�̃|�C���^�̂Ȃ����� */
	if (pInsertEvent->m_pPrevSameKindEvent) {
		pInsertEvent->m_pPrevSameKindEvent->m_pNextSameKindEvent = 
			MIDIEvent_SearchNextSameKindEvent (pInsertEvent->m_pPrevSameKindEvent);
	}
	if (pInsertEvent->m_pNextSameKindEvent) {
		pInsertEvent->m_pNextSameKindEvent->m_pPrevSameKindEvent =
			MIDIEvent_SearchPrevSameKindEvent (pInsertEvent->m_pNextSameKindEvent);
	}
	/* �O��̓���C�x���g�|�C���^�ݒ� */
	pInsertEvent->m_pPrevSameKindEvent = MIDIEvent_SearchPrevSameKindEvent (pInsertEvent);
	if (pInsertEvent->m_pPrevSameKindEvent) {
		pInsertEvent->m_pPrevSameKindEvent->m_pNextSameKindEvent = pInsertEvent;
	}
	pInsertEvent->m_pNextSameKindEvent = MIDIEvent_SearchNextSameKindEvent (pInsertEvent);
	if (pInsertEvent->m_pNextSameKindEvent) {
		pInsertEvent->m_pNextSameKindEvent->m_pPrevSameKindEvent = pInsertEvent;
	}
	/* �e�g���b�N�̃C�x���g����1��������B */
	pInsertEvent->m_pParent = pEvent->m_pParent;
	if (pEvent->m_pParent) {
		((MIDITrack*)(pEvent->m_pParent))->m_lNumEvent ++;
	}
	return 1;
}



/* �P��̃N���[���C�x���g�̍쐬 */
/* pEvent���g���b�N�ɑ����Ă��Ă����V�C�x���g�Ƃ��č쐬���܂��B */
/* (���̊֐��͓����B������Ă��܂��B) */
MIDIEvent* __stdcall MIDIEvent_CreateCloneSingle (MIDIEvent* pEvent) {
	MIDIEvent* pNewEvent = NULL;
	assert (pEvent);
	pNewEvent = calloc (sizeof (MIDIEvent), 1);
	if (pNewEvent == NULL) {
		return NULL;
	}
	pNewEvent->m_lTime = pEvent->m_lTime;
	pNewEvent->m_lKind = pEvent->m_lKind;
	if (!MIDIEvent_IsMIDIEvent (pEvent)) {
		if (pEvent->m_lLen > 0) { /* 20091024�������ǉ� */
			pNewEvent->m_pData = malloc (pEvent->m_lLen);
			if (pNewEvent->m_pData == NULL) {
				free (pNewEvent);
				return NULL;
			}
			memcpy (pNewEvent->m_pData, pEvent->m_pData, pEvent->m_lLen);
		}
		else { /* 20091024�������ǉ� */
			pNewEvent->m_pData = NULL;
			pNewEvent->m_lLen = 0;
		}
	}
	else {
		pNewEvent->m_pData = (unsigned char*)(&(pNewEvent->m_lData));
	}
	pNewEvent->m_lLen = pEvent->m_lLen;
	pNewEvent->m_lData = pEvent->m_lData;
	pNewEvent->m_lUser1 = pEvent->m_lUser1;
	pNewEvent->m_lUser2 = pEvent->m_lUser2;
	pNewEvent->m_lUser3 = pEvent->m_lUser3;
	pNewEvent->m_lUserFlag = pEvent->m_lUserFlag;
	pNewEvent->m_pParent = NULL;
	pNewEvent->m_pNextEvent = NULL;
	pNewEvent->m_pPrevEvent = NULL;
	pNewEvent->m_pNextSameKindEvent = NULL;
	pNewEvent->m_pPrevSameKindEvent = NULL;
	pNewEvent->m_pNextCombinedEvent = NULL;
	pNewEvent->m_pPrevCombinedEvent = NULL;
	return pNewEvent;
}


/*****************************************************************************/
/*                                                                           */
/*�@MIDIEvent�N���X�֐�                                                      */
/*                                                                           */
/*****************************************************************************/

/* �C�x���g���������� */
long __stdcall MIDIEvent_Combine (MIDIEvent* pEvent) {
	/* �m�[�g���F�m�[�g�I��+�m�[�g�I�t */
	/* �p�b�`�`�F���W���FCC#0+CC#32+PC */
	/* RPN�`�F���W���FCC#99+CC#98+CC#6 */
	/* NRPN�`�F���W���FCC#101+CC#100+CC#6 */
	assert (pEvent);
	/* ���Ɍ�������Ă�ꍇ�ُ͈�I�� */
	if (MIDIEvent_IsCombined (pEvent)) {
		return 0;
	}
	/* ����(a)��(b)�͓��ꃋ�[�v���ł͍��p���Ȃ��ł��������B */
	/* ����(a)��(b)�ɂ���Ă������񌋍�������Achop���Ȃ����肻��ȏ�͌����ł��܂���B */
	/* �m�[�g�I���C�x���g�Ƀm�[�g�I�t�C�x���g������(a) */
	if (MIDIEvent_IsNoteOn (pEvent)) {
		MIDIEvent* pNoteOff = pEvent;
		while ((pNoteOff = pNoteOff->m_pNextEvent)) {
			if (MIDIEvent_IsNoteOff (pNoteOff) && !MIDIEvent_IsCombined (pNoteOff)) {
				if (MIDIEvent_GetKey (pNoteOff) == MIDIEvent_GetKey (pEvent) &&
					MIDIEvent_GetChannel (pNoteOff) == MIDIEvent_GetChannel (pEvent)) {
					pEvent->m_pNextCombinedEvent = pNoteOff;
					pNoteOff->m_pPrevCombinedEvent = pEvent;
					return 1;
				}
			}
		}
		/* ���̃m�[�g�I���ɑΉ�����m�[�g�I�t��������Ȃ������B */
		return 0;
	}
	/* �m�[�g�I�t�C�x���g�Ƀm�[�g�I���C�x���g������(b) */
	else if (MIDIEvent_IsNoteOff (pEvent)) {
		MIDIEvent* pNoteOn = pEvent;
		while ((pNoteOn = pNoteOn->m_pPrevEvent)) {
			if (MIDIEvent_IsNoteOn (pNoteOn) && !MIDIEvent_IsCombined (pNoteOn)) {
				if (MIDIEvent_GetKey (pNoteOn) == MIDIEvent_GetKey (pEvent) &&
					MIDIEvent_GetChannel (pNoteOn) == MIDIEvent_GetChannel (pEvent)) {
					pEvent->m_pPrevCombinedEvent = pNoteOn;
					pNoteOn->m_pNextCombinedEvent = pEvent;
					return 1;
				}
			}
		}
		/* ���̃m�[�g�I�t�ɑΉ�����m�[�g�I����������Ȃ������B */
		return 0;
	}
	/* CC#6��CC#101��CC#100����CC#99��CC#98������ */
	else if (MIDIEvent_IsControlChange (pEvent)) {
		/* CC#6(�f�[�^�G���g���[)�Ɍ���T�� */
		if (MIDIEvent_GetNumber (pEvent) == 6) {
			MIDIEvent* p = pEvent;
			MIDIEvent* pCC100 = NULL;
			MIDIEvent* pCC101 = NULL;
			MIDIEvent* pCC98 = NULL;
			MIDIEvent* pCC99 = NULL;
			/* RPN�o���N�T�� */
			while ((p = p->m_pPrevEvent)) {
				if (MIDIEvent_GetTime (p) != MIDIEvent_GetTime (pEvent)) {
					break;
				}
				else if (MIDIEvent_IsControlChange (p)) {
					if (MIDIEvent_GetNumber (p) == 6) {
						break;
					}
				}
				if (MIDIEvent_IsControlChange (p) && !MIDIEvent_IsCombined (pEvent)) {
					if (MIDIEvent_GetNumber (p) == 100 && pCC100 == NULL &&
						MIDIEvent_GetChannel (p) == MIDIEvent_GetChannel (pEvent)) {
						pCC100 = p;
					}
					else if (MIDIEvent_GetNumber (p) == 101 && pCC101 == NULL && 
						MIDIEvent_GetChannel (p) == MIDIEvent_GetChannel (pEvent)) {
						pCC101 = p;
					}
				}
			}
			if (pCC100 && pCC101) {
				pCC101->m_pPrevCombinedEvent = NULL;
				pCC101->m_pNextCombinedEvent = pCC100;
				pCC100->m_pPrevCombinedEvent = pCC101;
				pCC100->m_pNextCombinedEvent = pEvent;
				pEvent->m_pPrevCombinedEvent = pCC101;
				pEvent->m_pNextCombinedEvent = NULL;
				return 2;
			}
			/* NRPN�o���N�T�� */
			p = pEvent;
			while ((p = p->m_pPrevEvent)) {
				if (MIDIEvent_GetTime (p) != MIDIEvent_GetTime (pEvent)) {
					break;
				}
				else if (MIDIEvent_IsControlChange (p)) {
					if (MIDIEvent_GetNumber (p) == 6) {
						break;
					}
				}
				
				if (MIDIEvent_IsControlChange (p) && !MIDIEvent_IsCombined (pEvent)) {
					if (MIDIEvent_GetNumber (p) == 98 && pCC98 == NULL &&
						MIDIEvent_GetChannel (p) == MIDIEvent_GetChannel (pEvent)) {
						pCC98 = p;
					}
					else if (MIDIEvent_GetNumber (p) == 99 && pCC99 == NULL &&
						MIDIEvent_GetChannel (p) == MIDIEvent_GetChannel (pEvent)) {
						pCC99 = p;
					}
				}
			}
			if (pCC98 && pCC99) {
				pCC99->m_pPrevCombinedEvent = NULL;
				pCC99->m_pNextCombinedEvent = pCC98;
				pCC98->m_pPrevCombinedEvent = pCC99;
				pCC98->m_pNextCombinedEvent = pEvent;
				pEvent->m_pPrevCombinedEvent = pCC98;
				pEvent->m_pNextCombinedEvent = NULL;
				return 2;
			}
			/* ����CC#6�ɑ΂���{CC#101��CC#100}����{CC#99��CC#98}��������Ȃ������B */
			return 0;
		}
		return 0;
	}
	/* �v���O�����`�F���W��CC#0��CC#32������ */
	else if (MIDIEvent_IsProgramChange (pEvent)) {
		MIDIEvent* p = pEvent;
		MIDIEvent* pCC0 = NULL;
		MIDIEvent* pCC32 = NULL;
		while ((p = p->m_pPrevEvent)) {
			if (MIDIEvent_GetTime (p) != MIDIEvent_GetTime (pEvent)) {
				break;
			}
			else if (MIDIEvent_IsProgramChange (p)) {
				break;
			}
			if (MIDIEvent_IsControlChange (p) && !MIDIEvent_IsCombined (pEvent)) {
				if (MIDIEvent_GetNumber (p) == 0 && pCC0 == NULL &&
					MIDIEvent_GetChannel (p) == MIDIEvent_GetChannel (pEvent)) {
					pCC0 = p;
				}
				else if (MIDIEvent_GetNumber (p) == 32 && pCC32 == NULL &&
					MIDIEvent_GetChannel (p) == MIDIEvent_GetChannel (pEvent)) {
					pCC32 = p;
				}
			}
		}
		if (pCC0 && pCC32) {
			pCC0->m_pPrevCombinedEvent = NULL;
			pCC0->m_pNextCombinedEvent = pCC32;
			pCC32->m_pPrevCombinedEvent = pCC0;
			pCC32->m_pNextCombinedEvent = pEvent;
			pEvent->m_pPrevCombinedEvent = pCC32;
			pEvent->m_pNextCombinedEvent = NULL;
			return 1;
		}
		/* ���̃v���O�����`�F���W�ɑ΂���CC#0��CC#32��������Ȃ������B */
		return 0;
	}
	/* �����ł���C�x���g�ł͂Ȃ������B */
	return 0;
}

/* �����C�x���g��؂藣�� */
long __stdcall MIDIEvent_Chop (MIDIEvent* pEvent) {
	long lCount = 0;
	MIDIEvent* pTempEvent = NULL;
	MIDIEvent* pExplodeEvent = NULL;
	/* �����C�x���g�łȂ��ꍇ�ُ͈�I�� */
	if (!MIDIEvent_IsCombined (pEvent)) {
		return 0;
	}
	/* �ŏ��̌������珇�Ԃɐ؂藣�� */
	pExplodeEvent = MIDIEvent_GetFirstCombinedEvent (pEvent);
	while (pExplodeEvent) {
		pTempEvent = pExplodeEvent->m_pNextCombinedEvent;
		pExplodeEvent->m_pPrevCombinedEvent = NULL;
		pExplodeEvent->m_pNextCombinedEvent = NULL;
		pExplodeEvent = pTempEvent;
		lCount++;
	}
	return lCount;
}

/* MIDI�C�x���g�̍폜(�������Ă���ꍇ�ł��P���MIDI�C�x���g���폜) */
long __stdcall MIDIEvent_DeleteSingle (MIDIEvent* pEvent) {
	if (pEvent == NULL) {
		return 0;
	}
	/* �f�[�^���̉�� */
	if (pEvent->m_pData != NULL && pEvent->m_pData != (unsigned char*)(&(pEvent->m_lData))) {
		free (pEvent->m_pData);
		pEvent->m_pData = NULL;
		pEvent->m_lLen = 0;
	}
	/* �����C�x���g�̐؂藣�� */
	if (pEvent->m_pNextCombinedEvent) {
		pEvent->m_pNextCombinedEvent->m_pPrevCombinedEvent = pEvent->m_pPrevCombinedEvent;
	}
	if (pEvent->m_pPrevCombinedEvent) {
		pEvent->m_pPrevCombinedEvent->m_pNextCombinedEvent = pEvent->m_pNextCombinedEvent;
	}
	/* �O��ڑ��|�C���^�̂Ȃ��ւ� */
	if (pEvent->m_pNextEvent) {
		pEvent->m_pNextEvent->m_pPrevEvent = pEvent->m_pPrevEvent;
	}
	else if (pEvent->m_pParent) {
		((MIDITrack*)(pEvent->m_pParent))->m_pLastEvent = pEvent->m_pPrevEvent;
	}
	if (pEvent->m_pPrevEvent) {
		pEvent->m_pPrevEvent->m_pNextEvent = pEvent->m_pNextEvent;
	}
	else if (pEvent->m_pParent) {
		((MIDITrack*)(pEvent->m_pParent))->m_pFirstEvent = pEvent->m_pNextEvent;
	}
	/* �O�㓯��C�x���g�ڑ��|�C���^�̂Ȃ��ւ� */
	if (pEvent->m_pNextSameKindEvent) {
		pEvent->m_pNextSameKindEvent->m_pPrevSameKindEvent = pEvent->m_pPrevSameKindEvent;
	}
	if (pEvent->m_pPrevSameKindEvent) {
		pEvent->m_pPrevSameKindEvent->m_pNextSameKindEvent = pEvent->m_pNextSameKindEvent;
	}
	/* �O�㌋���C�x���g�|�C���^�̂Ȃ��ւ� */
	if (pEvent->m_pNextCombinedEvent) {
		pEvent->m_pNextCombinedEvent->m_pPrevCombinedEvent = pEvent->m_pPrevCombinedEvent;
	}
	if (pEvent->m_pPrevCombinedEvent) {
		pEvent->m_pPrevCombinedEvent->m_pNextCombinedEvent = pEvent->m_pNextCombinedEvent;
	}
	/* ���̃C�x���g�̑��̃C�x���g�ւ̎Q�Ƃ����ׂ�NULL�� */
	pEvent->m_pNextEvent = NULL;
	pEvent->m_pPrevEvent = NULL;
	pEvent->m_pNextSameKindEvent = NULL;
	pEvent->m_pPrevSameKindEvent = NULL;
	pEvent->m_pNextCombinedEvent = NULL;
	pEvent->m_pPrevCombinedEvent = NULL;
	/* �e�g���b�N�̃C�x���g���f�N�������g */
	if (pEvent->m_pParent) {
		((MIDITrack*)(pEvent->m_pParent))->m_lNumEvent--;
	}
	pEvent->m_pParent = NULL;
	/* MIDI�C�x���g�I�u�W�F�N�g���̂̉�� */
	free (pEvent);
	pEvent = NULL;
	return 1;
}

/* MIDI�C�x���g�̍폜(�������Ă���ꍇ�A�������Ă���MIDI�C�x���g���폜) */
long __stdcall MIDIEvent_Delete (MIDIEvent* pEvent) {
	long lCount = 0;
	MIDIEvent* pDeleteEvent = pEvent;
	MIDIEvent* pTempEvent = NULL;
	assert (pEvent);
	pDeleteEvent = MIDIEvent_GetFirstCombinedEvent (pEvent);
	while (pDeleteEvent) {
		pTempEvent = pDeleteEvent->m_pNextCombinedEvent;
		MIDIEvent_DeleteSingle (pDeleteEvent);
		pDeleteEvent = pTempEvent;
		lCount++;
	}
	return lCount;
}

/* �w��C�x���g�Ɍ������Ă���C�x���g�̍폜 */
/* (���̊֐��͓����B������Ă��܂�) */
long __stdcall MIDIEvent_DeleteCombinedEvent (MIDIEvent* pEvent) {
	long lCount = 0;
	MIDIEvent* pDeleteEvent = NULL;
	MIDIEvent* pTempEvent = NULL;
	/* ���̃C�x���g���O�̌����C�x���g���폜 */
	pDeleteEvent = pEvent->m_pPrevCombinedEvent;
	while (pDeleteEvent) {
		pTempEvent = pDeleteEvent->m_pPrevCombinedEvent;
		MIDIEvent_DeleteSingle (pDeleteEvent);
		pDeleteEvent = pTempEvent;
		lCount++;
	}
	/* ���̃C�x���g����̌����C�x���g���폜 */
	pDeleteEvent = pEvent->m_pNextCombinedEvent;
	while (pDeleteEvent) {
		pTempEvent = pDeleteEvent->m_pNextCombinedEvent;
		MIDIEvent_DeleteSingle (pDeleteEvent);
		pDeleteEvent = pTempEvent;
		lCount++;
	}
	return lCount;
}


/* MIDI�C�x���g(�C��)�𐶐����AMIDI�C�x���g�ւ̃|�C���^��Ԃ�(���s��NULL) */
/* lTime:�}������[tick], lKind:�C�x���g�̎�� */
/* pData:�����f�[�^���ւ̃|�C���^, lLen:�f�[�^���̒���[�o�C�g](�ȉ����l) */
/* ���̊֐��̓����j���O�X�e�[�^�X���T�|�[�g���Ă��܂� */
/* pData�̍ŏ�1�o�C�g���������Ă��Ă�lKind�����pData�𕜌����܂��B */
MIDIEvent* __stdcall MIDIEvent_Create
(long lTime, long lKind, unsigned char* pData, long lLen) {
	MIDIEvent* pEvent;
	/* �����̐������`�F�b�N */
	if (lTime < 0) {
		return NULL;
	}
	if (lKind < 0 || lKind >= 256) {
		return NULL;
	}
	if (lLen < 0) {
		return NULL;
	}
	if (pData == NULL && lLen != 0) {
		return NULL;
	}
	/* MIDI�`�����l���C�x���g��3�o�C�g�ȉ��łȂ���΂Ȃ�Ȃ� */	
	if (0x80 <= lKind && lKind <= 0xEF && lLen >= 4) {
		return NULL;
	}
	/* MIDI�C�x���g�I�u�W�F�N�g�p�o�b�t�@�̊m�� */
	pEvent = calloc (sizeof (MIDIEvent), 1);
	if (pEvent == NULL) {
		return NULL;
	}
	/* �^�C���Ǝ�ނ̐ݒ� */
	pEvent->m_lTime = lTime;
	pEvent->m_lKind = lKind;
	/* pData�Ƀ����j���O�X�e�[�^�X���܂܂�Ă��Ȃ��ꍇ�̑[�u */
	if (((0x80 <= lKind && lKind <= 0xEF) && (0 <= *pData && *pData <= 127)) ||
		((lKind == 0xF0) && (0 <= *pData && *pData <= 127))) {
		lLen ++;
	}
	pEvent->m_lLen = lLen;
	/* MIDI�`�����l���C�x���g�̏ꍇ�́A�t����4�o�C�g�̃f�[�^�o�b�t�@���g���B */
	if (0x80 <= lKind && lKind <= 0xEF) {
		pEvent->m_pData = (unsigned char*)(&(pEvent->m_lData));
	}
	/* SysEx�����^�C�x���g�̏ꍇ�́A�O���Ƀf�[�^�o�b�t�@���m�ۂ���B */
	else {
		if (lLen > 0) {  /* 20091024�������ǉ� */
			pEvent->m_pData = malloc (lLen);
			if (pEvent->m_pData == NULL) {
				free (pEvent);
				return NULL;
			}
		}
		else {  /* 20091024�������ǉ� */
			pEvent->m_pData = NULL;
			pEvent->m_lLen = 0;
		}
	}
	/* �ڑ��|�C���^�̏����� */
	pEvent->m_pNextEvent = NULL;
	pEvent->m_pPrevEvent = NULL;
	pEvent->m_pNextSameKindEvent = NULL;
	pEvent->m_pPrevSameKindEvent = NULL;
	pEvent->m_pNextCombinedEvent = NULL;
	pEvent->m_pPrevCombinedEvent = NULL;
	pEvent->m_pParent = NULL;
	pEvent->m_lUser1 = 0;
	pEvent->m_lUser2 = 0;
	pEvent->m_lUser3 = 0;
	pEvent->m_lUserFlag = 0;
	/* �f�[�^���̐ݒ� */
	if (pData) {
		/* pData�Ƀ����j���O�X�e�[�^�X���܂܂�Ă��Ȃ��ꍇ�̑[�u */
		if (((0x80 <= lKind && lKind <= 0xEF) && (0 <= *pData && *pData <= 127)) ||
			((lKind == 0xF0) && (0 <= *pData && *pData <= 127))) {
			if (pEvent->m_pData != NULL) {
				memcpy (pEvent->m_pData, &lKind, 1);
			}
			if (pEvent->m_pData != NULL && lLen - 1 > 0) { /* 20091024�������ǉ� */
				memcpy (pEvent->m_pData + 1, pData, lLen - 1);
			}
		}
		/* �ʏ�̏ꍇ */
		else {
			/* MIDI�`�����l���C�x���g�̃C�x���g�̎�ނ̃`�����l�����́A�f�[�^���ɍ��킹�� */
			if (0x80 <= pEvent->m_lKind && pEvent->m_lKind <= 0xEF) {
				pEvent->m_lKind &= 0xF0;
				pEvent->m_lKind |= (*(pData) & 0x0F);
			}
			if (pEvent->m_pData != NULL && lLen > 0) { /* 20091024�������ǉ� */
				memcpy (pEvent->m_pData, pData, lLen);
			}
		}
	}
	return pEvent;

}


/* �N���[���C�x���g�̍쐬 */
/* pEvent�������C�x���g�̏ꍇ�A�S�����������C�x���g���쐬����B */
MIDIEvent* __stdcall MIDIEvent_CreateClone (MIDIEvent* pEvent) {
	long i = 0;
	long lPosition = 0;	
	MIDIEvent* pNewEvent = NULL;
	MIDIEvent* pSourceEvent = NULL;
	MIDIEvent* pPrevEvent = NULL;
	assert (pEvent);

	/* �����C�x���g�̏ꍇ�ŏ��̃C�x���g���擾 */
	pSourceEvent = pEvent;
	while (pSourceEvent->m_pPrevCombinedEvent) {
		pSourceEvent = pSourceEvent->m_pPrevCombinedEvent;
		lPosition++;
	}
	/* �ŏ��̃C�x���g���珇�ɂЂƂÂN���[�����쐬 */
	while (pSourceEvent) {
		pNewEvent = MIDIEvent_CreateCloneSingle (pSourceEvent);
		if (pNewEvent == NULL) {
			if (pPrevEvent) {
				MIDIEvent* pDeleteEvent = MIDIEvent_GetFirstCombinedEvent (pPrevEvent);
				MIDIEvent_Delete (pDeleteEvent);
			}
			return NULL;
		}
		/* �����C�x���g�|�C���^�̏��� */
		if (pPrevEvent) {
			pPrevEvent->m_pNextCombinedEvent = pNewEvent;
		}
		pNewEvent->m_pPrevCombinedEvent = pPrevEvent;
		pNewEvent->m_pNextCombinedEvent = NULL;
		/* ���̃C�x���g�֐i�߂� */
		pSourceEvent = pSourceEvent->m_pNextCombinedEvent;
		pPrevEvent = pNewEvent;
	}
	/* �߂�l�͐V�����쐬���������C�x���g��pEvent�ɑΉ�����C�x���g(20081124�ύX) */
	pNewEvent = MIDIEvent_GetFirstCombinedEvent (pNewEvent);
	for (i = 0; i < lPosition; i++) {
		pNewEvent = pNewEvent->m_pNextCombinedEvent;
	}
	return pNewEvent;
}

/* �V�[�P���X�i���o�[�C�x���g�̐��� */
MIDIEvent* __stdcall MIDIEvent_CreateSequenceNumber
(long lTime, long lNumber) {
	unsigned char c[2];
	c[0] = (unsigned char)((CLIP (0, lNumber, 65535) & 0xFF00) >> 8);
	c[1] = (unsigned char)(CLIP (0, lNumber, 65535) & 0x00FF);
	return MIDIEvent_Create (lTime, MIDIEVENT_SEQUENCENUMBER, 
		(unsigned char*)c, 2);
}

/* �e�L�X�g�C�x���g�̐���(�ėp) */
MIDIEvent* __stdcall MIDIEvent_CreateTextEvent
(long lTime, const char* pszText) {
	return MIDIEvent_Create (lTime, MIDIEVENT_TEXTEVENT, 
		(unsigned char*)pszText, strlen (pszText));
}

/* ���쌠�C�x���g�̐���(�ėp) */
MIDIEvent* __stdcall MIDIEvent_CreateCopyrightNotice
(long lTime, const char* pszText) {
	return MIDIEvent_Create (lTime, MIDIEVENT_COPYRIGHTNOTICE, 
		(unsigned char*)pszText, strlen (pszText));
}

/* �g���b�N���C�x���g�̐��� */
MIDIEvent* __stdcall MIDIEvent_CreateTrackName
(long lTime, const char* pszText) {
	return MIDIEvent_Create (lTime, MIDIEVENT_TRACKNAME, 
		(unsigned char*)pszText, strlen (pszText));
}

/* �C���X�g�D�������g�C�x���g�̐��� */
MIDIEvent* __stdcall MIDIEvent_CreateInstrumentName
(long lTime, const char* pszText) {
	return MIDIEvent_Create (lTime, MIDIEVENT_INSTRUMENTNAME,
		(unsigned char*)pszText, strlen (pszText));
}

/* �̎��C�x���g�̐��� */
MIDIEvent* __stdcall MIDIEvent_CreateLyric
(long lTime, const char* pszText) {
	return MIDIEvent_Create (lTime, MIDIEVENT_LYRIC, 
		(unsigned char*)pszText, strlen (pszText));
}

/* �}�[�J�[�C�x���g�̐��� */
MIDIEvent* __stdcall MIDIEvent_CreateMarker
(long lTime, const char* pszText) {
	return MIDIEvent_Create (lTime, MIDIEVENT_MARKER,
		(unsigned char*)pszText, strlen (pszText));
}

/* �L���[�|�C���g�C�x���g�̐��� */
MIDIEvent* __stdcall MIDIEvent_CreateCuePoint
(long lTime, const char* pszText) {
	return MIDIEvent_Create (lTime, MIDIEVENT_CUEPOINT, 
		(unsigned char*)pszText, strlen (pszText));
}

/* �v���O�������̐��� */
MIDIEvent* __stdcall MIDIEvent_CreateProgramName
(long lTime, const char* pszText) {
	return MIDIEvent_Create (lTime, MIDIEVENT_PROGRAMNAME, 
		(unsigned char*)pszText, strlen (pszText));
}

/* �f�o�C�X������ */
MIDIEvent* __stdcall MIDIEvent_CreateDeviceName
(long lTime, const char* pszText) {
	return MIDIEvent_Create (lTime, MIDIEVENT_DEVICENAME,
		(unsigned char*)pszText, strlen (pszText));
}

/* �`�����l���v���t�B�b�N�X�̐��� */
MIDIEvent* __stdcall MIDIEvent_CreateChannelPrefix
(long lTime, long lCh) {
	unsigned char c[1];
	c[0] = (unsigned char)(CLIP (0, lCh, 16));
	return MIDIEvent_Create (lTime, MIDIEVENT_CHANNELPREFIX, c, 1);
}

/* �|�[�g�v���t�B�b�N�X�̐��� */
MIDIEvent* __stdcall MIDIEvent_CreatePortPrefix
(long lTime, long lNum) {
	unsigned char c[1];
	c[0] = (unsigned char)(CLIP (0, lNum, 255));
	return MIDIEvent_Create (lTime, MIDIEVENT_PORTPREFIX, c, 1);
}

/* �G���h�I�u�g���b�N�C�x���g�̐��� */
MIDIEvent* __stdcall MIDIEvent_CreateEndofTrack (long lTime) {
	return MIDIEvent_Create (lTime, MIDIEVENT_ENDOFTRACK, NULL, 0);
}

/* �e���|�C�x���g�̐���(lTempo = 60000000/BPM�Ƃ���) */
MIDIEvent* __stdcall MIDIEvent_CreateTempo (long lTime, long lTempo) {
	unsigned char c[3];
	c[0] = (unsigned char)((CLIP (MIDIEVENT_MINTEMPO, lTempo, MIDIEVENT_MAXTEMPO) & 0xFF0000) >> 16);
	c[1] = (unsigned char)((CLIP (MIDIEVENT_MINTEMPO, lTempo, MIDIEVENT_MAXTEMPO) & 0x00FF00) >> 8);
	c[2] = (unsigned char)((CLIP (MIDIEVENT_MINTEMPO, lTempo, MIDIEVENT_MAXTEMPO) & 0x0000FF) >> 0);
	return MIDIEvent_Create (lTime, MIDIEVENT_TEMPO, c, 3);
}

/* SMPTE�I�t�Z�b�g�C�x���g�̐��� */
MIDIEvent* __stdcall MIDIEvent_CreateSMPTEOffset
(long lTime, long lMode, long lHour, long lMin, long lSec, long lFrame, long lSubFrame) {
	long lMaxFrame[4] = {23, 24, 29, 29};
	unsigned char c[5];
	c[0] = (unsigned char)(((lMode & 0x03) << 5) | (CLIP (0, lHour, 23)));
	c[1] = (unsigned char)(CLIP (0, lMin, 59));
	c[2] = (unsigned char)(CLIP (0, lSec, 59));
	c[3] = (unsigned char)(CLIP (0, lFrame, lMaxFrame[lMode & 0x03]));
	c[4] = (unsigned char)(CLIP (0, lSubFrame, 99));
	return MIDIEvent_Create (lTime, MIDIEVENT_SMPTEOFFSET, c, 5);
}

/* ���q�C�x���g�̐��� */
MIDIEvent* __stdcall MIDIEvent_CreateTimeSignature (long lTime, long lnn, long ldd, long lcc, long lbb) {
	unsigned char c[4];
	c[0] = (unsigned char)(CLIP (0, lnn, 255));
	c[1] = (unsigned char)(CLIP (0, ldd, 255));
	c[2] = (unsigned char)(CLIP (0, lcc, 255));
	c[3] = (unsigned char)(CLIP (0, lbb, 255));
	return MIDIEvent_Create (lTime, MIDIEVENT_TIMESIGNATURE, c, 4);
}

/* �����C�x���g�̐��� */
MIDIEvent* __stdcall MIDIEvent_CreateKeySignature (long lTime, long lsf, long lmi) {
	unsigned char c[2];
	c[0] = (unsigned char)(CLIP (-7, lsf, +7));
	c[1] = (unsigned char)(CLIP (0, lmi, 1));
	return MIDIEvent_Create (lTime, MIDIEVENT_KEYSIGNATURE, c, 2);
}

/* �V�[�P���T�Ǝ��̃C�x���g�̐��� */
MIDIEvent* __stdcall MIDIEvent_CreateSequencerSpecific (long lTime, char* pBuf, long lLen) {
	return MIDIEvent_Create (lTime, MIDIEVENT_SEQUENCERSPECIFIC, (unsigned char*)pBuf, lLen);
}

/* �m�[�g�I�t�C�x���g�̐��� */
MIDIEvent* __stdcall MIDIEvent_CreateNoteOff
(long lTime, long lCh, long lKey, long lVel) {
	unsigned char c[3];
	c[0] = (unsigned char)(MIDIEVENT_NOTEOFF | (lCh & 0x0F));
	c[1] = (unsigned char)(CLIP (0, lKey, 127));
	c[2] = (unsigned char)(CLIP (0, lVel, 127));
	return MIDIEvent_Create (lTime, MIDIEVENT_NOTEOFF | (lCh & 0x0F), c, 3);
}

/* �m�[�g�I���C�x���g�̐��� */
MIDIEvent* __stdcall MIDIEvent_CreateNoteOn
(long lTime, long lCh, long lKey, long lVel) {
	unsigned char c[3];
	c[0] = (unsigned char)(MIDIEVENT_NOTEON | (lCh & 0x0F));
	c[1] = (unsigned char)(CLIP (1, lKey, 127));
	c[2] = (unsigned char)(CLIP (0, lVel, 127));
	return MIDIEvent_Create (lTime, MIDIEVENT_NOTEON | (lCh & 0x0F), c, 3);
}

/* �m�[�g�C�x���g�̐��� */
/* (�m�[�g�I��(0x9n)�E�m�[�g�I�t(0x8n)��2�C�x���g�𐶐����ANoteOn�ւ̃|�C���^��Ԃ�) */
MIDIEvent* __stdcall MIDIEvent_CreateNoteOnNoteOff
(long lTime, long lCh, long lKey, long lVel1, long lVel2, long lDur) {
	unsigned char c[3];
	MIDIEvent* pNoteOnEvent;
	MIDIEvent* pNoteOffEvent;
	/* �m�[�g�I��(0x9n)�C�x���g�̐��� */
	c[0] = (unsigned char)(MIDIEVENT_NOTEON | (lCh & 0x0F));
	c[1] = (unsigned char)(CLIP (0, lKey, 127));
	c[2] = (unsigned char)(CLIP (1, lVel1, 127));
	pNoteOnEvent = MIDIEvent_Create (lTime, MIDIEVENT_NOTEON | (lCh & 0x0F), c, 3);
	if (pNoteOnEvent == NULL) {
		return NULL;
	}
	/* �m�[�g�I�t(0x8n)�C�x���g�̐��� */
	c[0] = (unsigned char)(MIDIEVENT_NOTEOFF | (lCh & 0x0F));
	c[1] = (unsigned char)(CLIP (0, lKey, 127));
	c[2] = (unsigned char)(CLIP (1, lVel2, 127));
	pNoteOffEvent = MIDIEvent_Create (lTime + lDur, MIDIEVENT_NOTEOFF | (lCh & 0x0F), c, 3);
	if (pNoteOffEvent == NULL) {
		MIDIEvent_DeleteSingle (pNoteOnEvent);
		return NULL;
	}
	/* ���2�C�x���g�̌��� */
	pNoteOnEvent->m_pPrevCombinedEvent = NULL;
	pNoteOnEvent->m_pNextCombinedEvent = pNoteOffEvent;
	pNoteOffEvent->m_pPrevCombinedEvent = pNoteOnEvent;
	pNoteOffEvent->m_pNextCombinedEvent = NULL;
	return pNoteOnEvent;
}

/* �m�[�g�C�x���g�̐��� */
/* (�m�[�g�I��(0x9n)�E�m�[�g�I��(0x9n(vel==0))��2�C�x���g�𐶐����ANoteOn�ւ̃|�C���^��Ԃ�) */
MIDIEvent* __stdcall MIDIEvent_CreateNoteOnNoteOn0
(long lTime, long lCh, long lKey, long lVel, long lDur) {
	unsigned char c[3];
	MIDIEvent* pNoteOnEvent;
	MIDIEvent* pNoteOffEvent;
	/* �m�[�g�I��(0x9n)�C�x���g�̐��� */
	c[0] = (unsigned char)(MIDIEVENT_NOTEON | (lCh & 0x0F));
	c[1] = (unsigned char)(CLIP (0, lKey, 127));
	c[2] = (unsigned char)(CLIP (1, lVel, 127));
	pNoteOnEvent = MIDIEvent_Create (lTime, MIDIEVENT_NOTEON | (lCh & 0x0F), c, 3);
	if (pNoteOnEvent == NULL) {
		return NULL;
	}
	/* �m�[�g�I��(0x9n, vel==0)�C�x���g�̐��� */
	c[0] = (unsigned char)(MIDIEVENT_NOTEON | (lCh & 0x0F));
	c[1] = (unsigned char)(CLIP (0, lKey, 127));
	c[2] = (unsigned char)0;
	pNoteOffEvent = MIDIEvent_Create (lTime + lDur, MIDIEVENT_NOTEON | (lCh & 0x0F), c, 3);
	if (pNoteOffEvent == NULL) {
		MIDIEvent_DeleteSingle (pNoteOnEvent);
		return NULL;
	}
	/* ���2�C�x���g�̌��� */
	pNoteOnEvent->m_pPrevCombinedEvent = NULL;
	pNoteOnEvent->m_pNextCombinedEvent = pNoteOffEvent;
	pNoteOffEvent->m_pPrevCombinedEvent = pNoteOnEvent;
	pNoteOffEvent->m_pNextCombinedEvent = NULL;
	return pNoteOnEvent;
}

/* �m�[�g�C�x���g�̐��� */
/* (�m�[�g�I���E�m�[�g������2�C�x���g�𐶐����ANoteOn�ւ̃|�C���^��Ԃ�) */
MIDIEvent* __stdcall MIDIEvent_CreateNote
	(long lTime, long lCh, long lKey, long lVel, long lDur) {
	return MIDIEvent_CreateNoteOnNoteOn0 (lTime, lCh, lKey, lVel, lDur);
}

/* �L�[�A�t�^�[�C�x���g�̐��� */
MIDIEvent* __stdcall MIDIEvent_CreateKeyAftertouch 
(long lTime, long lCh, long lKey, long lVal) {
	unsigned char c[3];
	c[0] = (unsigned char)(MIDIEVENT_KEYAFTERTOUCH | (lCh & 0x0F));
	c[1] = (unsigned char)(CLIP (0, lKey, 127));
	c[2] = (unsigned char)(CLIP (0, lVal, 127));
	return MIDIEvent_Create (lTime, MIDIEVENT_KEYAFTERTOUCH | c[0], c, 3);
}

/* �R���g���[���[�C�x���g�̐��� */
MIDIEvent* __stdcall MIDIEvent_CreateControlChange
(long lTime, long lCh, long lNum, long lVal) {
	unsigned char c[3];
	c[0] = (unsigned char)(MIDIEVENT_CONTROLCHANGE | (lCh & 0x0F));
	c[1] = (unsigned char)(CLIP (0, lNum, 127));
	c[2] = (unsigned char)(CLIP (0, lVal, 127));
	return MIDIEvent_Create (lTime, MIDIEVENT_CONTROLCHANGE | c[0], c, 3);
}

/* RPN�C�x���g�̐��� */
/* (CC#101+CC#100+CC#6��3�C�x���g�𐶐����ACC#101�ւ̃|�C���^��Ԃ�) */
/* (lBank�ɂ�(CC#101*128+CC#100)�̒l���w�肷��) */
MIDIEvent* __stdcall MIDIEvent_CreateRPNChange
(long lTime, long lCh, long lBank, long lVal) {
	MIDIEvent* pCC101Event;
	MIDIEvent* pCC100Event;
	MIDIEvent* pCC6Event;
	long lCC101Val = (CLIP (0, lBank, 16383) & 0x3F80) >> 7; /* 20081123�C�� */
	long lCC100Val = (CLIP (0, lBank, 16383) & 0x007F); /* 20081123�C�� */
	/* CC#101�C�x���g�̐��� */
	pCC101Event = MIDIEvent_CreateControlChange (lTime, lCh, 101, lCC101Val);
	if (pCC101Event == NULL) {
		return NULL;
	}
	/* CC#100�C�x���g�̐��� */
	pCC100Event = MIDIEvent_CreateControlChange (lTime, lCh, 100, lCC100Val);
	if (pCC100Event == NULL) {
		MIDIEvent_DeleteSingle (pCC101Event);
		return NULL;
	}
	/* CC#6(�f�[�^�G���g���[)�C�x���g�̐��� */
	pCC6Event = MIDIEvent_CreateControlChange (lTime, lCh, 6, lVal);
	if (pCC6Event == NULL) {
		MIDIEvent_DeleteSingle (pCC101Event);
		MIDIEvent_DeleteSingle (pCC100Event);
		return NULL;
	}
	/* ���3�C�x���g�̌��� */
	pCC101Event->m_pPrevCombinedEvent = NULL;
	pCC101Event->m_pNextCombinedEvent = pCC100Event;
	pCC100Event->m_pPrevCombinedEvent = pCC101Event;
	pCC100Event->m_pNextCombinedEvent = pCC6Event;
	pCC6Event->m_pPrevCombinedEvent = pCC100Event;
	pCC6Event->m_pNextCombinedEvent = NULL;
	return pCC101Event;
}

/* NRPN�C�x���g�̐��� */
/* (CC#99+CC#98+CC#6��3�C�x���g�𐶐����ACC#99�ւ̃|�C���^��Ԃ�) */
/* (lBank�ɂ�(CC#99*128+CC#98)�̒l���w�肷��) */
MIDIEvent* __stdcall MIDIEvent_CreateNRPNChange 
(long lTime, long lCh, long lBank, long lVal) {
	MIDIEvent* pCC99Event;
	MIDIEvent* pCC98Event;
	MIDIEvent* pCC6Event;
	long lCC99Val = (CLIP (0, lBank, 16383) & 0x3F80) >> 7; /* 20081123�C�� */
	long lCC98Val = (CLIP (0, lBank, 16383) & 0x007F); /* 20081123�C�� */
	/* CC#99�C�x���g�̐��� */
	pCC99Event = MIDIEvent_CreateControlChange (lTime, lCh, 99, lCC99Val);
	if (pCC99Event == NULL) {
		return NULL;
	}
	/* CC#98�C�x���g�̐��� */
	pCC98Event = MIDIEvent_CreateControlChange (lTime, lCh, 98, lCC98Val);
	if (pCC98Event == NULL) {
		MIDIEvent_DeleteSingle (pCC99Event);
		return NULL;
	}
	/* CC#6(�f�[�^�G���g���[)�C�x���g�̐��� */
	pCC6Event = MIDIEvent_CreateControlChange (lTime, lCh, 6, lVal);
	if (pCC6Event == NULL) {
		MIDIEvent_DeleteSingle (pCC99Event);
		MIDIEvent_DeleteSingle (pCC98Event);
		return NULL;
	}
	/* ���3�C�x���g�̌��� */
	pCC99Event->m_pPrevCombinedEvent = NULL;
	pCC99Event->m_pNextCombinedEvent = pCC98Event;
	pCC98Event->m_pPrevCombinedEvent = pCC99Event;
	pCC98Event->m_pNextCombinedEvent = pCC6Event;
	pCC6Event->m_pPrevCombinedEvent = pCC98Event;
	pCC6Event->m_pNextCombinedEvent = NULL;
	return pCC99Event;
}

/* �v���O�����C�x���g�̐��� */
MIDIEvent* __stdcall MIDIEvent_CreateProgramChange 
(long lTime, long lCh, long lVal) {
	unsigned char c[2];
	c[0] = (unsigned char)(MIDIEVENT_PROGRAMCHANGE | (lCh & 0x0F));
	c[1] = (unsigned char)(CLIP (0, lVal, 127));
	return MIDIEvent_Create (lTime, MIDIEVENT_PROGRAMCHANGE | c[0], c, 2);
}

/* �p�b�`�`�F���W�C�x���g�̐��� */
/* (CC#32+CC#0+PC��3�C�x���g�𐶐����ACC#32�ւ̃|�C���^��Ԃ�) */
/* (lBank�ɂ�(CC#32*128+CC#0)�̒l���w�肷��) */
MIDIEvent* __stdcall MIDIEvent_CreatePatchChange 
(long lTime, long lCh, long lBank, long lVal) {
	MIDIEvent* pCC32Event;
	MIDIEvent* pCC0Event;
	MIDIEvent* pProgramEvent;
	long lCC0Val = (CLIP (0, lBank, 16383) & 0x3F10) >> 7; /* 20081124 �C�� */
	long lCC32Val = (CLIP (0, lBank, 16383) & 0x007F);
	/* CC#0�C�x���g�̐��� */
	pCC0Event = MIDIEvent_CreateControlChange (lTime, lCh, 0, lCC0Val);
	if (pCC0Event == NULL) {
		return NULL;
	}
	/* CC#32�C�x���g�̐��� */
	pCC32Event = MIDIEvent_CreateControlChange (lTime, lCh, 32, lCC32Val);
	if (pCC32Event == NULL) {
		MIDIEvent_DeleteSingle (pCC0Event);
		return NULL;
	}
	/* �v���O�����`�F���W�C�x���g�̐��� */
	pProgramEvent = MIDIEvent_CreateProgramChange (lTime, lCh, lVal);
	if (pProgramEvent == NULL) {
		MIDIEvent_DeleteSingle (pCC0Event);
		MIDIEvent_DeleteSingle (pCC32Event);
		return NULL;
	}
	/* ���3�C�x���g�̌��� */
	pCC0Event->m_pPrevCombinedEvent = NULL;
	pCC0Event->m_pNextCombinedEvent = pCC32Event;
	pCC32Event->m_pPrevCombinedEvent = pCC0Event;
	pCC32Event->m_pNextCombinedEvent = pProgramEvent;
	pProgramEvent->m_pPrevCombinedEvent = pCC32Event;
	pProgramEvent->m_pNextCombinedEvent = NULL;
	return pCC0Event;
}

/* �`�����l���A�t�^�[�C�x���g�̐��� */
MIDIEvent* __stdcall MIDIEvent_CreateChannelAftertouch 
(long lTime, long lCh, long lVal) {
	unsigned char c[2];
	c[0] = (unsigned char)(MIDIEVENT_CHANNELAFTERTOUCH | (lCh & 0x0F));
	c[1] = (unsigned char)(CLIP (0, lVal, 127));
	return MIDIEvent_Create (lTime, MIDIEVENT_CHANNELAFTERTOUCH | c[0], c, 2);
}


/* �s�b�`�x���h�C�x���g�̐��� */
MIDIEvent* __stdcall MIDIEvent_CreatePitchBend 
(long lTime, long lCh, long lVal) {
	unsigned char c[3];
	c[0] = (unsigned char)(MIDIEVENT_PITCHBEND | (lCh & 0x0F));
	c[1] = (unsigned char)(CLIP (0, lVal, 16383) & 0x7F);
	c[2] = (unsigned char)((CLIP (0, lVal, 16383) >> 7) & 0x7F);
	return MIDIEvent_Create (lTime, MIDIEVENT_PITCHBEND | c[0], c, 3);
}

/* SysEx�C�x���g�̐��� */
MIDIEvent* __stdcall MIDIEvent_CreateSysExEvent
(long lTime, unsigned char* pBuf, long lLen) {
	assert (pBuf);
	if (*pBuf == 0xF0) {
		return MIDIEvent_Create (lTime, MIDIEVENT_SYSEXSTART, pBuf, lLen);
	}
	else {
		return MIDIEvent_Create (lTime, MIDIEVENT_SYSEXCONTINUE, pBuf, lLen);
	}
}





/* ���^�C�x���g�ł��邩�ǂ����𒲂ׂ� */
/* ���^�C�x���g�Ƃ́A�C�x���g�̎�ނ�0x00�`0x7F�̂��́A���Ȃ킿�A */
/* �V�[�P���X�ԍ��E�e�L�X�g�C�x���g�E���쌠�E�g���b�N���E */
/* �C���X�g�D�������g���E�̎��E�}�[�J�[�E�L���[�|�C���g�E */
/* �v���O�������E�f�o�C�X���E�`�����l���v���t�B�b�N�X�E�|�[�g�v���t�B�b�N�X�E */
/* �G���h�I�u�g���b�N�E�e���|�ESMPTE�I�t�Z�b�g�E���q�L���E�����L���E�V�[�P���T�[�Ǝ��̃C�x���g */
/* �Ȃǂ������B�����͎�ɕ\�L�����̂��߂̃C�x���g�ł���A���t�ɉe����^������̂ł͂Ȃ��B */
long __stdcall MIDIEvent_IsMetaEvent (MIDIEvent* pEvent) {
	return (MIDIEVENT_SEQUENCENUMBER <= pEvent->m_lKind && 
		pEvent->m_lKind <= MIDIEVENT_SEQUENCERSPECIFIC) ? 1 : 0;
}

/* �V�[�P���X�ԍ��ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsSequenceNumber (MIDIEvent* pEvent) {
	return (pEvent->m_lKind == MIDIEVENT_SEQUENCENUMBER) ? 1 : 0;
}

/* �e�L�X�g�C�x���g�ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsTextEvent (MIDIEvent* pEvent) {
	return (pEvent->m_lKind == MIDIEVENT_TEXTEVENT) ? 1 : 0;
}

/* ���쌠�C�x���g�ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsCopyrightNotice (MIDIEvent* pEvent) {
	return (pEvent->m_lKind == MIDIEVENT_COPYRIGHTNOTICE) ? 1 : 0;
}

/* �g���b�N���C�x���g�ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsTrackName (MIDIEvent* pEvent) {
	return (pEvent->m_lKind == MIDIEVENT_TRACKNAME) ? 1 : 0;
}

/* �C���X�g�D�������g���ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsInstrumentName (MIDIEvent* pEvent) {
	return (pEvent->m_lKind == MIDIEVENT_INSTRUMENTNAME) ? 1 : 0;
}

/* �̎��C�x���g�ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsLyric (MIDIEvent* pEvent) {
	return (pEvent->m_lKind == MIDIEVENT_LYRIC) ? 1 : 0;
}

/* �}�[�J�[�C�x���g�ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsMarker (MIDIEvent* pEvent) {
	return (pEvent->m_lKind == MIDIEVENT_MARKER) ? 1 : 0;
}

/* �L���[�|�C���g�C�x���g�ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsCuePoint (MIDIEvent* pEvent) {
	return (pEvent->m_lKind == MIDIEVENT_CUEPOINT) ? 1 : 0;
}

/* �v���O�������ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsProgramName (MIDIEvent* pEvent) {
	return (pEvent->m_lKind == MIDIEVENT_PROGRAMNAME) ? 1 : 0;
}

/* �f�o�C�X���ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsDeviceName (MIDIEvent* pEvent) {
	return (pEvent->m_lKind == MIDIEVENT_DEVICENAME) ? 1 : 0;
}

/* �`�����l���v���t�B�b�N�X�ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsChannelPrefix (MIDIEvent* pEvent) {
	return (pEvent->m_lKind == MIDIEVENT_CHANNELPREFIX) ? 1 : 0;
}

/* �|�[�g�v���t�B�b�N�X�ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsPortPrefix (MIDIEvent* pEvent) {
	return pEvent->m_lKind == MIDIEVENT_PORTPREFIX ? 1 : 0;
}

/* �G���h�I�u�g���b�N�ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsEndofTrack (MIDIEvent* pEvent) {
	return (pEvent->m_lKind == MIDIEVENT_ENDOFTRACK) ? 1 : 0;
}

/* �e���|�C�x���g�ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsTempo (MIDIEvent* pEvent) {
	return pEvent->m_lKind == MIDIEVENT_TEMPO ? 1 : 0;
}

/* SMPTE�I�t�Z�b�g�C�x���g�ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsSMPTEOffset (MIDIEvent* pEvent) {
	return pEvent->m_lKind == MIDIEVENT_SMPTEOFFSET ? 1 : 0;
}

/* ���q�L���C�x���g�ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsTimeSignature (MIDIEvent* pEvent) {
	return pEvent->m_lKind == MIDIEVENT_TIMESIGNATURE ? 1 : 0;
}

/* �����L���C�x���g�ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsKeySignature (MIDIEvent* pEvent) {
	return pEvent->m_lKind == MIDIEVENT_KEYSIGNATURE ? 1 : 0;
}

/* �V�[�P���T�Ǝ��̃C�x���g�ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsSequencerSpecific (MIDIEvent* pEvent) {
	return pEvent->m_lKind == MIDIEVENT_SEQUENCERSPECIFIC ? 1 : 0;
}

/* MIDI�C�x���g�ł��邩�ǂ����𒲂ׂ� */
/* MIDI�C�x���g�Ƃ̓m�[�g�I�t�E�I�[�g�I���E�L�[�A�t�^�[�^�b�`�E�R���g���[���`�F���W�E */
/* �v���O�����`�F���W�E�`�����l���A�t�^�[�^�b�`�E�s�b�`�x���h�A�y�т����̌������� */
/* �����C�x���g�̂��Ƃ������B */
long __stdcall MIDIEvent_IsMIDIEvent (MIDIEvent* pEvent) {
	return (0x80 <= pEvent->m_lKind && pEvent->m_lKind <= 0xEF) ? 1 : 0;
}

/* NOTE�C�x���g�ł��邩�ǂ����𒲂ׂ� */
/* ����̓m�[�g�I���ƃm�[�g�I�t�������C�x���g�����C�x���g�łȂ���΂Ȃ�Ȃ��B */
long __stdcall MIDIEvent_IsNote (MIDIEvent* pEvent) {
	MIDIEvent* pNoteOnEvent = NULL;
	MIDIEvent* pNoteOffEvent = NULL;
	if (!MIDIEvent_IsCombined (pEvent)) {
		return 0;
	}
	pNoteOnEvent = MIDIEvent_GetFirstCombinedEvent (pEvent);
	if (pNoteOnEvent == NULL) {
		return 0;
	}
	if (!MIDIEvent_IsNoteOn (pNoteOnEvent)) {
		return 0;
	}
	pNoteOffEvent = pNoteOnEvent->m_pNextCombinedEvent;
	if (pNoteOffEvent == NULL) {
		return 0;
	}
	if (!MIDIEvent_IsNoteOff (pNoteOffEvent)) {
		return 0;
	}
	if (pNoteOffEvent->m_pNextCombinedEvent) {
		return 0;
	}
	return 1;
}

/* NOTEONOTEOFF�C�x���g�ł��邩�ǂ����𒲂ׂ� */
/* ����̓m�[�g�I��(0x9n)�ƃm�[�g�I�t(0x8n)�������C�x���g�����C�x���g�łȂ���΂Ȃ�Ȃ��B */
long __stdcall MIDIEvent_IsNoteOnNoteOff (MIDIEvent* pEvent) {
	MIDIEvent* pNoteOnEvent = NULL;
	MIDIEvent* pNoteOffEvent = NULL;
	if (!MIDIEvent_IsCombined (pEvent)) {
		return 0;
	}
	pNoteOnEvent = MIDIEvent_GetFirstCombinedEvent (pEvent);
	if (pNoteOnEvent == NULL) {
		return 0;
	}
	if (!MIDIEvent_IsNoteOn (pNoteOnEvent)) {
		return 0;
	}
	pNoteOffEvent = pNoteOnEvent->m_pNextCombinedEvent;
	if (pNoteOffEvent == NULL) {
		return 0;
	}
	if (!(0x80 <= pNoteOffEvent->m_lKind && pNoteOffEvent->m_lKind <= 0x8F)) {
		return 0;
	}
	if (pNoteOffEvent->m_pNextCombinedEvent) {
		return 0;
	}
	return 1;
}

/* NOTEONNOTEON0�C�x���g�ł��邩�ǂ����𒲂ׂ� */
/* ����̓m�[�g�I��(0x9n)�ƃm�[�g�I�t(0x9n,vel==0)�������C�x���g�����C�x���g�łȂ���΂Ȃ�Ȃ��B */
long __stdcall MIDIEvent_IsNoteOnNoteOn0 (MIDIEvent* pEvent) {
	MIDIEvent* pNoteOnEvent = NULL;
	MIDIEvent* pNoteOffEvent = NULL;
	if (!MIDIEvent_IsCombined (pEvent)) {
		return 0;
	}
	pNoteOnEvent = MIDIEvent_GetFirstCombinedEvent (pEvent);
	if (pNoteOnEvent == NULL) {
		return 0;
	}
	if (!MIDIEvent_IsNoteOn (pNoteOnEvent)) {
		return 0;
	}
	pNoteOffEvent = pNoteOnEvent->m_pNextCombinedEvent;
	if (pNoteOffEvent == NULL) {
		return 0;
	}
	if (!(0x90 <= pNoteOffEvent->m_lKind && pNoteOffEvent->m_lKind <= 0x9F)) {
		return 0;
	}
	if (*(pNoteOffEvent->m_pData + 2) != 0) {
		return 0;
	}
	if (pNoteOffEvent->m_pNextCombinedEvent) {
		return 0;
	}
	return 1;
}

/* �m�[�g�I�t�C�x���g�ł��邩�ǂ����𒲂ׂ� */
/* (�m�[�g�I���C�x���g�Ńx���V�e�B0�̂��̂̓m�[�g�I�t�C�x���g�Ƃ݂Ȃ��B) */
long __stdcall MIDIEvent_IsNoteOff (MIDIEvent* pEvent) {
	if (0x80 <= pEvent->m_lKind && pEvent->m_lKind <= 0x8F) {
		return 1;
	}
	if (0x90 <= pEvent->m_lKind && pEvent->m_lKind <= 0x9F) {
		if (*(pEvent->m_pData + 2)) {
			return 0;
		}
		return 1;
	}
	return 0;
}

/* �m�[�g�I���C�x���g�ł��邩�ǂ����𒲂ׂ� */
/* (�m�[�g�I���C�x���g�ł��x���V�e�B0�̂��̂̓m�[�g�I�t�C�x���g�Ƃ݂Ȃ��B) */
long __stdcall MIDIEvent_IsNoteOn (MIDIEvent* pEvent) {
	if (0x90 <= pEvent->m_lKind && pEvent->m_lKind <= 0x9F) {
		if (*(pEvent->m_pData + 2)) {
			return 1;
		}
		return 0;
	}
	return 0;
}

/* �L�[�A�t�^�[�^�b�`�C�x���g�ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsKeyAftertouch (MIDIEvent* pEvent) {
	return (0xA0 <= pEvent->m_lKind && pEvent->m_lKind <= 0xAF) ? 1 : 0;
}

/* �R���g���[���`�F���W�C�x���g�ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsControlChange (MIDIEvent* pEvent) {
	return (0xB0 <= pEvent->m_lKind && pEvent->m_lKind <= 0xBF) ? 1 : 0;
}

/* �v���O�����`�F���W�C�x���g�ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsProgramChange (MIDIEvent* pEvent) {
	return (0xC0 <= pEvent->m_lKind && pEvent->m_lKind <= 0xCF) ? 1 : 0;
}

/* �`�����l���A�t�^�[�C�x���g�ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsChannelAftertouch (MIDIEvent* pEvent) {
	return (0xD0 <= pEvent->m_lKind && pEvent->m_lKind <= 0xDF) ? 1 : 0;
}

/* �s�b�`�x���h�C�x���g�ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsPitchBend (MIDIEvent* pEvent) {
	return (0xE0 <= pEvent->m_lKind && pEvent->m_lKind <= 0xEF) ? 1 : 0;
}

/* �V�X�e���G�N�X�N���[�V���C�x���g�ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDIEvent_IsSysExEvent (MIDIEvent* pEvent) {
	return (pEvent->m_lKind == 0xF0 || pEvent->m_lKind == 0xF7) ? 1 : 0;
}

/* ���V�C�x���g�ł��邩�ǂ������ׂ� */
long __stdcall MIDIEvent_IsFloating (MIDIEvent* pEvent) {
	return (pEvent->m_pParent == NULL ? 1 : 0);
}

/* �����C�x���g�ł��邩�ǂ������ׂ� */
long __stdcall MIDIEvent_IsCombined (MIDIEvent* pEvent) {
	return (pEvent->m_pPrevCombinedEvent || pEvent->m_pNextCombinedEvent) ? 1 : 0;
}

/* RPN�`�F���W�C�x���g�ł��邩�ǂ����𒲂ׂ� */
/* CC#101��CC#100��CC#6�����������̏��ɂŌ�������ĂȂ���΂Ȃ�Ȃ��B */
long __stdcall MIDIEvent_IsRPNChange (MIDIEvent* pEvent) {
	MIDIEvent* pCC101Event = NULL;
	MIDIEvent* pCC100Event = NULL;
	MIDIEvent* pCC006Event = NULL;
	assert (pEvent);
	if (!MIDIEvent_IsCombined (pEvent)) {
		return 0;
	}
	pCC101Event = MIDIEvent_GetFirstCombinedEvent (pEvent);
	if (pCC101Event == NULL) {
		return 0;
	}
	if (pCC101Event->m_lTime != pEvent->m_lTime) {
		return 0;
	}
	if (!MIDIEvent_IsControlChange (pCC101Event)) {
		return 0;
	}
	if (*(pCC101Event->m_pData + 1) != 101) {
		return 0;
	}		
	pCC100Event = pCC101Event->m_pNextCombinedEvent;
	if (pCC100Event == NULL) {
		return 0;
	}
	if (pCC100Event->m_lTime != pEvent->m_lTime) {
		return 0;
	}
	if (!MIDIEvent_IsControlChange (pCC100Event)) {
		return 0;
	}
	if (*(pCC100Event->m_pData + 1) != 100) {
		return 0;
	}
	pCC006Event = pCC100Event->m_pNextCombinedEvent;
	if (pCC006Event == NULL) {
		return 0;
	}
	if (pCC006Event->m_lTime != pEvent->m_lTime) {
		return 0;
	}
	if (!MIDIEvent_IsControlChange (pCC006Event)) {
		return 0;
	}
	if (pCC006Event->m_pNextCombinedEvent) {
		return 0;
	}
	return 1;
}


/* NRPN�`�F���W�C�x���g�ł��邩�ǂ����𒲂ׂ� */
/* CC#99��CC#98��CC#6�����������̏��ԂŌ�������Ă��Ȃ���΂Ȃ�Ȃ��B */
long __stdcall MIDIEvent_IsNRPNChange (MIDIEvent* pEvent) {
	MIDIEvent* pCC099Event = NULL;
	MIDIEvent* pCC098Event = NULL;
	MIDIEvent* pCC006Event = NULL;
	assert (pEvent);
	if (!MIDIEvent_IsCombined (pEvent)) {
		return 0;
	}
	pCC099Event = MIDIEvent_GetFirstCombinedEvent (pEvent);
	if (pCC099Event == NULL) {
		return 0;
	}
	if (pCC099Event->m_lTime != pEvent->m_lTime) {
		return 0;
	}
	if (!MIDIEvent_IsControlChange (pCC099Event)) {
		return 0;
	}
	if (*(pCC099Event->m_pData + 1) != 99) {
		return 0;
	}		
	pCC098Event = pCC099Event->m_pNextCombinedEvent;
	if (pCC098Event == NULL) {
		return 0;
	}
	if (pCC098Event->m_lTime != pEvent->m_lTime) {
		return 0;
	}
	if (!MIDIEvent_IsControlChange (pCC098Event)) {
		return 0;
	}
	if (*(pCC098Event->m_pData + 1) != 98) {
		return 0;
	}
	pCC006Event = pCC098Event->m_pNextCombinedEvent;
	if (pCC006Event == NULL) {
		return 0;
	}
	if (pCC006Event->m_lTime != pEvent->m_lTime) {
		return 0;
	}
	if (!MIDIEvent_IsControlChange (pCC006Event)) {
		return 0;
	}
	if (pCC006Event->m_pNextCombinedEvent) {
		return 0;
	}
	return 1;
}


/* �p�b�`�`�F���W�C�x���g�ł��邩�ǂ����𒲂ׂ� */
/* CC#0��CC#32�ƃv���O�����`�F���W���������ɂ��̏��ԂŌ�������Ă��Ȃ���΂Ȃ�Ȃ��B */
long __stdcall MIDIEvent_IsPatchChange (MIDIEvent* pEvent) {
	MIDIEvent* pCC000Event = NULL;
	MIDIEvent* pCC032Event = NULL;
	MIDIEvent* pPCEvent = NULL;
	assert (pEvent);
	if (!MIDIEvent_IsCombined (pEvent)) {
		return 0;
	}
	pCC000Event = MIDIEvent_GetFirstCombinedEvent (pEvent);
	if (pCC000Event == NULL) {
		return 0;
	}
	if (pCC000Event->m_lTime != pEvent->m_lTime) {
		return 0;
	}
	if (!MIDIEvent_IsControlChange (pCC000Event)) {
		return 0;
	}
	if (*(pCC000Event->m_pData + 1) != 0) {
		return 0;
	}		
	pCC032Event = pCC000Event->m_pNextCombinedEvent;
	if (pCC032Event == NULL) {
		return 0;
	}
	if (pCC032Event->m_lTime != pEvent->m_lTime) {
		return 0;
	}
	if (!MIDIEvent_IsControlChange (pCC032Event)) {
		return 0;
	}
	if (*(pCC032Event->m_pData + 1) != 32) {
		return 0;
	}
	pPCEvent = pCC032Event->m_pNextCombinedEvent;
	if (pPCEvent == NULL) {
		return 0;
	}
	if (pPCEvent->m_lTime != pEvent->m_lTime) {
		return 0;
	}
	if (!MIDIEvent_IsProgramChange (pPCEvent)) {
		return 0;
	}
	if (pPCEvent->m_pNextCombinedEvent) {
		return 0;
	}
	return 1;
}


/* �C�x���g�̎�ނ��擾 */
long __stdcall MIDIEvent_GetKind (MIDIEvent* pEvent) {
	/* TODO MIDI�`�����l���C�x���g�̏ꍇ�� & 0xF0 */
	return (pEvent->m_lKind);
}


long __stdcall MIDIEvent_SetKindSingle (MIDIEvent* pEvent, long lKind) {
	long lLen = 0;
	long lSrcKind;
	assert (pEvent);
	assert (0 <= lKind && lKind <= 0xFF);
	/* MIDI�`�����l���C�x���g�ւ̕ύX�̓`�����l����񖳎� */
	if (0x80 <= lKind && lKind <= 0xEF) {
		lKind &= 0xF0;
	}
	/* �C�x���g�̎�ނ��ύX����Ȃ��Ƃ��̓��^�[��0 */
	lSrcKind = pEvent->m_lKind;
	if (0x80 <= lSrcKind && lSrcKind <= 0xEF) {
		lSrcKind &= 0xF0;
	}
	if (lSrcKind == lKind) {
		return 0;
	}
	/* �o�b�t�@�̒����ݒ� */
	switch (lKind) {
	case MIDIEVENT_ENDOFTRACK:
		lLen = 0;
		break;
	case MIDIEVENT_CHANNELPREFIX:
	case MIDIEVENT_PORTPREFIX:
		lLen = 1;
		break;
	case MIDIEVENT_SEQUENCENUMBER:
	case MIDIEVENT_CHANNELAFTERTOUCH:
	case MIDIEVENT_PROGRAMCHANGE:
	case MIDIEVENT_KEYSIGNATURE:
	case MIDIEVENT_SYSEXSTART:
	case MIDIEVENT_SYSEXCONTINUE:
		lLen = 2;
		break;
	case MIDIEVENT_NOTEOFF:
	case MIDIEVENT_NOTEON:
	case MIDIEVENT_CONTROLCHANGE:
	case MIDIEVENT_PITCHBEND:
	case MIDIEVENT_KEYAFTERTOUCH:
	case MIDIEVENT_TEMPO:
		lLen = 3;
		break;
	case MIDIEVENT_TIMESIGNATURE:
		lLen = 4;
		break;
	case MIDIEVENT_SMPTEOFFSET:
		lLen = 5;
		break;
	default:
		lLen = pEvent->m_lLen;
		break;
	}
	/* �ύX��C�x���g��MIDI�`�����l���C�x���g�̏ꍇ */
	if (0x80 <= lKind && lKind <= 0xEF) {
		/* �f�[�^�p�������̉�� */
		if (pEvent->m_pData != (unsigned char*)(&(pEvent->m_lData))) {
			free (pEvent->m_pData);
			pEvent->m_pData = NULL;
			pEvent->m_lLen = 0;
		}
		pEvent->m_pData = (unsigned char*)(&(pEvent->m_lData));
		pEvent->m_lLen = lLen;
		pEvent->m_lKind = (lKind & 0xF0) | (pEvent->m_lKind & 0x0F);
		*(pEvent->m_pData) = (unsigned char)((lKind & 0xF0) | (pEvent->m_lKind & 0x0F));
	}
	/* �ύX��C�x���g�����^�C�x���g���V�X�e���G�N�X�N���[�V�u�C�x���g�̏ꍇ */
	else {
		long lMode = 0;
		long lMaxFrame[4] = {23, 24, 29, 29};
		/* �f�[�^�p�������̊m�� */
		/* �f�[�^����malloc�o�b�t�@���w���Ă���ꍇ */
		if (pEvent->m_pData != (unsigned char*)(&(pEvent->m_lData))) {
			if (lLen > 0) {
				if (pEvent->m_lLen != lLen) {
					pEvent->m_pData = realloc (pEvent->m_pData, lLen);
					if (pEvent->m_pData == NULL) {
						pEvent->m_lLen = 0;
						/* TODO:�����Ɉ��S�[�u���L�q */
					}
					else {
						pEvent->m_lLen = lLen;
					}
				}
			}
			/* lLen == 0�̏ꍇ */
			else {
				/* �f�[�^�p�������͂���Ȃ� */
				free (pEvent->m_pData);
				pEvent->m_pData = NULL;
				pEvent->m_lLen = 0;
			}
		}
		/* �f�[�^����long�o�b�t�@���w���Ă���ꍇ */
		else {
			if (lLen > 0) { /* 20091024�������ǉ� */
				/* 4�o�C�g�������͖������A�f�[�^�p��������V���Ɋm�� */
				pEvent->m_pData = malloc (lLen);
				if (pEvent->m_pData == NULL) {
					pEvent->m_lLen = 0;
					/* TODO:�����Ɉ��S�[�u���L�q */
				}
				else {
					pEvent->m_lLen = lLen;
				}
			}
			else {
				pEvent->m_pData = NULL;
				pEvent->m_lLen = 0;
			}
		}
		pEvent->m_lKind = lKind;
		/* �f�[�^���̐������m�� */
		switch (lKind) {
		case MIDIEVENT_CHANNELPREFIX:
			*(pEvent->m_pData) = (unsigned char)(CLIP (0, *(pEvent->m_pData), 15));
			break;
		case MIDIEVENT_PORTPREFIX:
			*(pEvent->m_pData) = (unsigned char)(CLIP (0, *(pEvent->m_pData), 255));
			break;
		case MIDIEVENT_SMPTEOFFSET:
			*(pEvent->m_pData) =  (unsigned char)((*(pEvent->m_pData) & 0x60) | 
				CLIP (0, (*(pEvent->m_pData) & 0x1F), 23));
			lMode = (*(pEvent->m_pData) >> 5) & 0x03;
			*(pEvent->m_pData + 1) = (unsigned char)CLIP (0, *(pEvent->m_pData + 1), 59);
			*(pEvent->m_pData + 2) = (unsigned char)CLIP (0, *(pEvent->m_pData + 2), 59);
			*(pEvent->m_pData + 3) = (unsigned char)CLIP (0, *(pEvent->m_pData + 3), lMaxFrame[lMode]);
			*(pEvent->m_pData + 4) = (unsigned char)CLIP (0, *(pEvent->m_pData + 4), 99);
			break;
		case MIDIEVENT_TIMESIGNATURE:
			*(pEvent->m_pData + 0) = (unsigned char)4;
			*(pEvent->m_pData + 1) = (unsigned char)2;
			*(pEvent->m_pData + 2) = (unsigned char)24;
			*(pEvent->m_pData + 3) = (unsigned char)8;
			break;
		case MIDIEVENT_KEYSIGNATURE:
			*(pEvent->m_pData + 0) = (unsigned char)CLIP (-7, *(pEvent->m_pData), +7);
			*(pEvent->m_pData + 1) = (unsigned char)CLIP (0, *(pEvent->m_pData + 1), 1);
			break;
		case MIDIEVENT_SYSEXSTART:
		case MIDIEVENT_SYSEXCONTINUE:
			*(pEvent->m_pData + 0) = (unsigned char)0xF0;
			*(pEvent->m_pData + 1) = (unsigned char)0xF7;
			break;
		}
	}
	/* �O��̓���C�x���g�̃|�C���^�̂Ȃ��ւ� */
	if (pEvent->m_pPrevSameKindEvent) {
		pEvent->m_pPrevSameKindEvent->m_pNextSameKindEvent = 
			MIDIEvent_SearchNextSameKindEvent (pEvent->m_pPrevSameKindEvent);
	}
	if (pEvent->m_pNextSameKindEvent) {
		pEvent->m_pNextSameKindEvent->m_pPrevSameKindEvent =
			MIDIEvent_SearchPrevSameKindEvent (pEvent->m_pNextSameKindEvent);
	}
	/* �O��̓���C�x���g�|�C���^�ݒ� */
	pEvent->m_pPrevSameKindEvent = MIDIEvent_SearchPrevSameKindEvent (pEvent);
	if (pEvent->m_pPrevSameKindEvent) {
		pEvent->m_pPrevSameKindEvent->m_pNextSameKindEvent = pEvent;
	}
	pEvent->m_pNextSameKindEvent = MIDIEvent_SearchNextSameKindEvent (pEvent);
	if (pEvent->m_pNextSameKindEvent) {
		pEvent->m_pNextSameKindEvent->m_pPrevSameKindEvent = pEvent;
	}
	return 1;
}


/* �C�x���g�̎�ނ�ݒ� */
/* TODO:���̊֐��͂܂�m_pNextSameKindEvent��m_pPrevSameKind�̐ݒ肪�Ȃ���Ă��Ȃ� */
long __stdcall MIDIEvent_SetKind (MIDIEvent* pEvent, long lKind) {
	assert (pEvent);
	/* MIDI�`�����l���C�x���g�̏ꍇ�A�`�����l�����͖��� */
	if (0x0080 <= (lKind & 0x00FF) && (lKind & 0x00FF) <= 0x00EF) {
		lKind &= 0xFFFFFFF0;
	}
	/* �C�x���g�̎�ނɕω����Ȃ��ꍇ�͒E�o */
	if (MIDIEvent_IsCombined (pEvent)) {
		if ((MIDIEvent_IsNoteOnNoteOff (pEvent) && lKind == MIDIEVENT_NOTEONNOTEOFF) ||
			(MIDIEvent_IsNoteOnNoteOn0 (pEvent) && lKind == MIDIEVENT_NOTEONNOTEON0)) {
			return 0;
		}
		else if ((MIDIEvent_IsPatchChange (pEvent) && lKind == MIDIEVENT_PATCHCHANGE) ||
				(MIDIEvent_IsRPNChange (pEvent) && lKind == MIDIEVENT_RPNCHANGE) ||
				(MIDIEvent_IsNRPNChange (pEvent) && lKind == MIDIEVENT_NRPNCHANGE)) {
			return 0;
		}
	}
	/* 2��2 */
	if ((MIDIEvent_IsNoteOnNoteOff (pEvent) && lKind == MIDIEVENT_NOTEONNOTEON0) ||
		(MIDIEvent_IsNoteOnNoteOn0 (pEvent) && lKind == MIDIEVENT_NOTEONNOTEOFF)) {
		MIDIEvent* pEvent1 = MIDIEvent_GetFirstCombinedEvent (pEvent);
		MIDIEvent* pEvent2 = pEvent1->m_pNextCombinedEvent;
		assert (MIDIEvent_GetChannel (pEvent1) == MIDIEvent_GetChannel (pEvent2));
		if (lKind == MIDIEVENT_NOTEONNOTEOFF) {
			pEvent2->m_lKind = (unsigned char)
				(MIDIEVENT_NOTEOFF | ((pEvent2->m_lKind) & 0x0F));
			*(pEvent2->m_pData + 0) = (unsigned char)
				(MIDIEVENT_NOTEOFF | ((pEvent2->m_lKind) & 0x0F));
			*(pEvent2->m_pData + 2) = (unsigned char)
				(*(pEvent1->m_pData + 2));
		}
		else if (lKind == MIDIEVENT_NOTEONNOTEON0) {
			pEvent2->m_lKind = (unsigned char)
				(MIDIEVENT_NOTEON | ((pEvent2->m_lKind) & 0x0F));
			*(pEvent2->m_pData + 0) = (unsigned char)
				(MIDIEVENT_NOTEON | ((pEvent2->m_lKind) & 0x0F));
			*(pEvent2->m_pData + 2) = (unsigned char)(0);
		}
	}
	/* 3��3 */
	else if ((MIDIEvent_IsRPNChange (pEvent) || MIDIEvent_IsNRPNChange (pEvent) || 
		MIDIEvent_IsPatchChange (pEvent)) && (lKind == MIDIEVENT_RPNCHANGE || 
		lKind == MIDIEVENT_NRPNCHANGE || lKind == MIDIEVENT_PATCHCHANGE)) {
		MIDIEvent* pEvent1 = MIDIEvent_GetFirstCombinedEvent (pEvent);
		MIDIEvent* pEvent2 = pEvent1->m_pNextCombinedEvent;
		MIDIEvent* pEvent3 = pEvent2->m_pNextCombinedEvent;
		if (lKind == MIDIEVENT_RPNCHANGE) {
			*(pEvent1->m_pData + 1) = 101;
			*(pEvent1->m_pData + 2) = (unsigned char)
				CLIP (0, *(pEvent1->m_pData + 2), 127);
			*(pEvent2->m_pData + 1) = 100;
			*(pEvent2->m_pData + 2) = (unsigned char)
				CLIP (0, *(pEvent2->m_pData + 2), 127);
			MIDIEvent_SetKindSingle (pEvent3, 0xB0 | (pEvent->m_lKind & 0x0F));
			*(pEvent3->m_pData + 1) = (unsigned char)6;
			*(pEvent3->m_pData + 2) = (unsigned char)
				CLIP (0, *(pEvent3->m_pData + 2), 127);
		}
		else if (lKind == MIDIEVENT_NRPNCHANGE) {
			*(pEvent1->m_pData + 1) = 99;
			*(pEvent1->m_pData + 2) = (unsigned char)
				CLIP (0, *(pEvent1->m_pData + 2), 127);
			*(pEvent2->m_pData + 1) = 98;
			*(pEvent2->m_pData + 2) = (unsigned char)
				CLIP (0, *(pEvent2->m_pData + 2), 127);
			MIDIEvent_SetKindSingle (pEvent3, 0xB0 | (pEvent->m_lKind & 0x0F));
			*(pEvent3->m_pData + 1) = (unsigned char)6;
			*(pEvent3->m_pData + 2) = (unsigned char)
				CLIP (0, *(pEvent3->m_pData + 2), 127);
		}
		else if (lKind == MIDIEVENT_PATCHCHANGE) {
			*(pEvent1->m_pData + 1) = 0;
			*(pEvent1->m_pData + 2) = (unsigned char)
				CLIP (0, *(pEvent1->m_pData + 2), 127);
			*(pEvent2->m_pData + 1) = 32;
			*(pEvent2->m_pData + 2) = (unsigned char)
				CLIP (0, *(pEvent2->m_pData + 2), 127);
			MIDIEvent_SetKindSingle (pEvent3, 0xC0 | (pEvent->m_lKind & 0x0F));
			*(pEvent3->m_pData + 1) = (unsigned char)
				CLIP (0, *(pEvent3->m_pData + 1), 127);
		}
	}
	/* 1��2, 3��2 */
	else if (lKind == MIDIEVENT_NOTEONNOTEOFF || lKind == MIDIEVENT_NOTEONNOTEON0) {
		MIDIEvent* pEvent2 = NULL;
		MIDIEvent_DeleteCombinedEvent (pEvent);
		MIDIEvent_SetKindSingle (pEvent, 0x90 | (pEvent->m_lKind & 0x0F));
		*(pEvent->m_pData + 1) =  (unsigned char)60;
		*(pEvent->m_pData + 2) =  (unsigned char)100;
		if (lKind == MIDIEVENT_NOTEONNOTEOFF) {
			pEvent2 = MIDIEvent_CreateNoteOff 
				(pEvent->m_lTime, (pEvent->m_lKind & 0x0F), 60, 100);
		}
		else {
			pEvent2 = MIDIEvent_CreateNoteOn 
				(pEvent->m_lTime, (pEvent->m_lKind & 0x0F), 60, 0);
		}
		pEvent->m_pPrevCombinedEvent = NULL;
		pEvent->m_pNextCombinedEvent = pEvent2;
		pEvent2->m_pPrevCombinedEvent = pEvent;
		pEvent2->m_pNextCombinedEvent = NULL;
		if (!MIDIEvent_IsFloating (pEvent)) {
			MIDIEvent_SetNextEvent (pEvent, pEvent2);
		}
	}
	/* 1��3, 2��3 */
	else if (lKind == MIDIEVENT_RPNCHANGE || lKind == MIDIEVENT_NRPNCHANGE ||
		lKind == MIDIEVENT_PATCHCHANGE) {
		MIDIEvent* pEvent2 = NULL;
		MIDIEvent* pEvent3 = NULL;
		MIDIEvent_DeleteCombinedEvent (pEvent);
		if (lKind == MIDIEVENT_NRPNCHANGE) {
			MIDIEvent_SetKindSingle (pEvent, 0xB0 | (pEvent->m_lKind & 0x0F));
			*(pEvent->m_pData + 1) = (unsigned char)99;
			*(pEvent->m_pData + 2) = (unsigned char)0;
			pEvent2 = MIDIEvent_CreateControlChange 
				(pEvent->m_lTime, (pEvent->m_lKind & 0x0F), 98, 0);
			pEvent3 = MIDIEvent_CreateControlChange 
				(pEvent->m_lTime, (pEvent->m_lKind & 0x0F), 06, 64);
		}
		else if (lKind == MIDIEVENT_RPNCHANGE) {
			MIDIEvent_SetKindSingle (pEvent, 0xB0 | (pEvent->m_lKind & 0x0F));
			*(pEvent->m_pData + 1) = (unsigned char)101;
			*(pEvent->m_pData + 2) = (unsigned char)0;
			pEvent2 = MIDIEvent_CreateControlChange 
				(pEvent->m_lTime, (pEvent->m_lKind & 0x0F), 100, 0);
			pEvent3 = MIDIEvent_CreateControlChange 
				(pEvent->m_lTime, (pEvent->m_lKind & 0x0F), 06, 64);
		}
		else if (lKind == MIDIEVENT_PATCHCHANGE) {
			MIDIEvent_SetKindSingle (pEvent, 0xC0 | (pEvent->m_lKind & 0x0F));
			*(pEvent->m_pData + 1) = (unsigned char)0;
			*(pEvent->m_pData + 2) = (unsigned char)0;
			pEvent2 = MIDIEvent_CreateControlChange 
				(pEvent->m_lTime, (pEvent->m_lKind & 0x0F), 32, 0);
			pEvent3 = MIDIEvent_CreateProgramChange 
				(pEvent->m_lTime, (pEvent->m_lKind & 0x0F), 0);
		}
		pEvent->m_pPrevCombinedEvent = NULL;
		pEvent->m_pNextCombinedEvent = pEvent2;
		pEvent2->m_pPrevCombinedEvent = pEvent;
		pEvent2->m_pNextCombinedEvent = pEvent3;
		pEvent3->m_pPrevCombinedEvent = pEvent2;
		pEvent3->m_pNextCombinedEvent = NULL;
		if (!MIDIEvent_IsFloating (pEvent)) {
			MIDIEvent_SetNextEvent (pEvent, pEvent2);
			MIDIEvent_SetNextEvent (pEvent2, pEvent3);
		}
	}
	/* 1��1, 2��1, 3��1 */
	else {
		MIDIEvent_DeleteCombinedEvent (pEvent);
		MIDIEvent_SetKindSingle (pEvent, (lKind & 0x00FF));
	}
	return 1;
}

/* �C�x���g�̒����擾 */
long __stdcall MIDIEvent_GetLen (MIDIEvent* pEvent) {
	return (pEvent->m_lLen);
}

/* �C�x���g�̃f�[�^�����擾 */
long __stdcall MIDIEvent_GetData (MIDIEvent* pEvent, unsigned char* pBuf, long lLen) {
	assert (pEvent);
	assert (pBuf);
	assert (0 <= lLen && lLen < MIDIEVENT_MAXLEN);
	memset (pBuf, 0, lLen);
	if (pEvent->m_pData != NULL && pEvent->m_lLen > 0) { /* 20091024�������ǉ� */
		memcpy (pBuf, pEvent->m_pData, MIN (lLen, pEvent->m_lLen));
	}
	return MIN (lLen, pEvent->m_lLen);
}

/* �C�x���g�̃f�[�^����ݒ�(���̊֐��͑�ϊ댯�ł��B�������̃`�F�L�͂��܂���) */
long __stdcall MIDIEvent_SetData (MIDIEvent* pEvent, unsigned char* pBuf, long lLen) {
	assert (pEvent);
	assert (pBuf);
	assert (0 <= lLen && lLen < MIDIEVENT_MAXLEN);
	if (MIDIEvent_IsMIDIEvent (pEvent)) {
		if (lLen <= 0 || lLen >= 4) {
			return 0;
		}
		if (pEvent->m_pData) { /* 20091024�������ǉ� */
			pEvent->m_lLen = lLen;
			memcpy (pEvent->m_pData, pBuf, lLen);
		}
	}
	else {
		if (lLen > 0) { /* 20091024�������ǉ� */
			pEvent->m_pData = realloc (pEvent->m_pData, lLen);
			if (pEvent->m_pData == NULL) {
				pEvent->m_lLen = 0;
				return 0;
			}
			else {
				pEvent->m_lLen = lLen;
				memcpy (pEvent->m_pData, pBuf, lLen);
			}
		}
		else { /* 20091024�������ǉ� */
			free (pEvent->m_pData);
			pEvent->m_pData = NULL;
			pEvent->m_lLen = 0;
		}
	}
	return 1;
}

/* �C�x���g�̃e�L�X�g���擾 */
char* __stdcall MIDIEvent_GetText (MIDIEvent* pEvent, char* pBuf, long lLen) {
	assert (pEvent);
	assert (pBuf);
	assert (0 < lLen && lLen < MIDIEVENT_MAXLEN);
	if (pEvent->m_lKind <= 0x00 || pEvent->m_lKind >= 0x1F) {
		return 0;
	}
	memset (pBuf, 0, lLen);
	if (pEvent->m_pData && pEvent->m_lLen > 0) { /* 20091024�������ǉ� */
		memcpy (pBuf, pEvent->m_pData, MIN (lLen - 1, pEvent->m_lLen));
	}
	return pBuf;
}

/* �C�x���g�̃e�L�X�g��ݒ� */
long __stdcall MIDIEvent_SetText (MIDIEvent* pEvent, const char* pszText) {
	long lLen;
	assert (pEvent);
	assert (pszText);
	if (pEvent->m_lKind <= 0x00 || pEvent->m_lKind >= 0x1F) {
		return 0;
	}
	lLen = strlen (pszText);
	if (lLen > 0) { /* 20091024�������ǉ� */
		pEvent->m_pData = realloc (pEvent->m_pData, lLen);
		if (pEvent->m_pData == NULL) {
			pEvent->m_lLen = 0;
			return 0;
		}
		else {
			pEvent->m_lLen = lLen;
			memcpy (pEvent->m_pData, pszText, lLen);
		}
	}
	else { /* 20091024�������ǉ� */
		free (pEvent->m_pData);
		pEvent->m_pData = NULL;
		pEvent->m_lLen = 0;
	}
	return 1;
}

/* SMPTE�I�t�Z�b�g�̎擾(SMPTE�I�t�Z�b�g�C�x���g�̂�) */
long __stdcall MIDIEvent_GetSMPTEOffset 
(MIDIEvent* pEvent, long* pMode, long* pHour, long* pMin, long* pSec, long* pFrame, long* pSubFrame) {
	assert (pEvent);
	assert (pMode);
	assert (pHour);
	assert (pMin);
	assert (pSec);
	assert (pFrame);
	assert (pSubFrame);
	if (pEvent->m_lKind != MIDIEVENT_SMPTEOFFSET) {
		return 0;
	}
	*pMode =     *(pEvent->m_pData) >> 5;
	*pHour =     *(pEvent->m_pData) & 0x1F;
	*pMin =      *(pEvent->m_pData + 1);
	*pSec =      *(pEvent->m_pData + 2);
	*pFrame =    *(pEvent->m_pData + 3);
	*pSubFrame = *(pEvent->m_pData + 4);
	return 1;
}

/* SMPTE�I�t�Z�b�g�̐ݒ�(SMPTE�I�t�Z�b�g�C�x���g�̂�) */
long __stdcall MIDIEvent_SetSMPTEOffset 
(MIDIEvent* pEvent, long lMode, long lHour, long lMin, long lSec, long lFrame, long lSubFrame) {
	long lMaxFrame[4] = {23, 24, 29, 29};
	assert (pEvent);
	if (pEvent->m_lKind != MIDIEVENT_SMPTEOFFSET) {
		return 0;
	}
	*(pEvent->m_pData) = (unsigned char)(((lMode & 0x03) << 5) | (CLIP (0, lHour, 23)));
	*(pEvent->m_pData + 1) = (unsigned char)(CLIP (0, lMin, 59));
	*(pEvent->m_pData + 2) = (unsigned char)(CLIP (0, lSec, 59));
	*(pEvent->m_pData + 3) = (unsigned char)(CLIP (0, lFrame, lMaxFrame[lMode & 0x03]));
	*(pEvent->m_pData + 4) = (unsigned char)(CLIP (0, lSubFrame, 99));
	return 1;
}

/* �e���|�擾(�e���|�C�x���g�̂�) */
long __stdcall MIDIEvent_GetTempo (MIDIEvent* pEvent) {
	assert (pEvent);
	if (pEvent->m_lKind != MIDIEVENT_TEMPO) {
		return 0;
	}
	return *(pEvent->m_pData) << 16 | *(pEvent->m_pData + 1) << 8 | *(pEvent->m_pData + 2);
}

/* �e���|�ݒ�(�e���|�C�x���g�̂�) */
long __stdcall MIDIEvent_SetTempo (MIDIEvent* pEvent, long lTempo) {
	unsigned char c[3];
	assert (pEvent);
	if (pEvent->m_lKind != MIDIEVENT_TEMPO) {
		return 0;
	}
	c[0] = (unsigned char)((CLIP (MIDIEVENT_MINTEMPO, lTempo, MIDIEVENT_MAXTEMPO) & 0xFF0000) >> 16);
	c[1] = (unsigned char)((CLIP (MIDIEVENT_MINTEMPO, lTempo, MIDIEVENT_MAXTEMPO) & 0x00FF00) >> 8);
	c[2] = (unsigned char)((CLIP (MIDIEVENT_MINTEMPO, lTempo, MIDIEVENT_MAXTEMPO) & 0x0000FF) >> 0);
	memcpy (pEvent->m_pData, c, 3);
	return 1;
}

/* ���q�擾(���q�L���C�x���g�̂�) */
long __stdcall MIDIEvent_GetTimeSignature (MIDIEvent* pEvent, long* pnn, long* pdd, long* pcc, long* pbb) {
	assert (pEvent);
	assert (pnn);
	assert (pdd);
	assert (pcc);
	assert (pbb);
	if (pEvent->m_lKind != MIDIEVENT_TIMESIGNATURE) {
		return 0;
	}
	*pnn = (long)(*(pEvent->m_pData));
	*pdd = (long)(*(pEvent->m_pData + 1));
	*pcc = (long)(*(pEvent->m_pData + 2));
	*pbb = (long)(*(pEvent->m_pData + 3));
	return 1;
}

/* ���q�̐ݒ�(���q�L���C�x���g�̂�) */
long __stdcall MIDIEvent_SetTimeSignature (MIDIEvent* pEvent, long lnn, long ldd, long lcc, long lbb) {
	assert (pEvent);
	if (pEvent->m_lKind != MIDIEVENT_TIMESIGNATURE) {
		return 0;
	}
	*(pEvent->m_pData) = (unsigned char)lnn;
	*(pEvent->m_pData + 1) = (unsigned char)ldd;
	*(pEvent->m_pData + 2) = (unsigned char)lcc;
	*(pEvent->m_pData + 3) = (unsigned char)lbb;
	return 1;
}

/* �����L���̎擾(�����L���C�x���g�̂�) */
long __stdcall MIDIEvent_GetKeySignature (MIDIEvent* pEvent, long* psf, long* pmi) {
	assert (pEvent);
	assert (psf);
	assert (pmi);
	if (pEvent->m_lKind != MIDIEVENT_KEYSIGNATURE) {
		return 0;
	}
	*psf = (long)(char)(*(pEvent->m_pData));
	*pmi = (long)(*(pEvent->m_pData + 1));
	return 1;
}

/* �����L���̐ݒ�(�����L���C�x���g�̂�) */
long __stdcall MIDIEvent_SetKeySignature (MIDIEvent* pEvent, long lsf, long lmi) {
	assert (pEvent);
	if (pEvent->m_lKind != MIDIEVENT_KEYSIGNATURE) {
		return 0;
	}
	*(pEvent->m_pData) = (unsigned char)CLIP (-7, lsf, 7);
	*(pEvent->m_pData + 1) = (unsigned char)CLIP (0, lmi, 1);
	return 1;
}

/* �C�x���g�̃��b�Z�[�W�擾(MIDI�C�x���g�y�уV�X�e���G�N�X�N���[�V���̂�) */
long __stdcall MIDIEvent_GetMIDIMessage (MIDIEvent* pEvent, char* pMessage, long lLen) {
	assert (pEvent);
	assert (pMessage);
	assert (lLen > 0);
	if (MIDIEvent_IsMIDIEvent (pEvent) || MIDIEvent_IsSysExEvent (pEvent)) {
		return MIDIEvent_GetData (pEvent, (unsigned char*)pMessage, lLen);
	}
	return 0;
}

/* �C�x���g�̃��b�Z�[�W�ݒ�(MIDI�C�x���g�y�уV�X�e���G�N�X�N���[�V���̂�) */
long __stdcall MIDIEvent_SetMIDIMessage (MIDIEvent* pEvent, char* pMessage, long lLen) {
	assert (pEvent);
	assert (pMessage);
	assert (lLen > 0);
	if (MIDIEvent_IsMIDIEvent (pEvent) || MIDIEvent_IsSysExEvent (pEvent)) {
		return MIDIEvent_SetData (pEvent, (unsigned char*)pMessage, lLen);
	}
	return 0;
}

/* �C�x���g�̃`�����l���擾(MIDI�C�x���g�̂�) */
long __stdcall MIDIEvent_GetChannel (MIDIEvent* pEvent) {
	assert (pEvent);
	assert (0x80 <= pEvent->m_lKind && pEvent->m_lKind <= 0xEF);
	if (MIDIEvent_IsMIDIEvent (pEvent)) {
		assert (pEvent->m_lKind  == *(pEvent->m_pData));
		return pEvent->m_lKind & 0x0F;
	}
	return 0;
}

/* �C�x���g�̃`�����l���ݒ�(MIDI�C�x���g�̂�) */
long __stdcall MIDIEvent_SetChannel (MIDIEvent* pEvent, long lCh) {
	long lCount = 0;
	MIDIEvent* pTempEvent = NULL;
	assert (pEvent);
	assert (0x80 <= pEvent->m_lKind && pEvent->m_lKind <= 0xEF);
	pTempEvent = MIDIEvent_GetFirstCombinedEvent (pEvent);
	while (pTempEvent) {
		if (MIDIEvent_IsMIDIEvent (pTempEvent))	{
			pTempEvent->m_lKind &= 0xF0;
			pTempEvent->m_lKind |= (unsigned char)(CLIP (0, lCh, 15));
			*(pTempEvent->m_pData) &= 0xF0;
			*(pTempEvent->m_pData) |= (unsigned char)(CLIP (0, lCh, 15));
			assert (pTempEvent->m_lKind == *(pTempEvent->m_pData));
			/* �O��̓���C�x���g�̃|�C���^�̂Ȃ��ւ� */
			if (pTempEvent->m_pPrevSameKindEvent) {
				pTempEvent->m_pPrevSameKindEvent->m_pNextSameKindEvent = 
					MIDIEvent_SearchNextSameKindEvent (pTempEvent->m_pPrevSameKindEvent);
			}
			if (pTempEvent->m_pNextSameKindEvent) {
				pTempEvent->m_pNextSameKindEvent->m_pPrevSameKindEvent =
					MIDIEvent_SearchPrevSameKindEvent (pTempEvent->m_pNextSameKindEvent);
			}
			/* �O��̓���C�x���g�|�C���^�ݒ� */
			pTempEvent->m_pPrevSameKindEvent = MIDIEvent_SearchPrevSameKindEvent (pTempEvent);
			if (pTempEvent->m_pPrevSameKindEvent) {
				pTempEvent->m_pPrevSameKindEvent->m_pNextSameKindEvent = pTempEvent;
			}
			pTempEvent->m_pNextSameKindEvent = MIDIEvent_SearchNextSameKindEvent (pTempEvent);
			if (pTempEvent->m_pNextSameKindEvent) {
				pTempEvent->m_pNextSameKindEvent->m_pPrevSameKindEvent = pTempEvent;
			}
			lCount++;
		}
		pTempEvent = pTempEvent->m_pNextCombinedEvent;
	};
	return lCount;
}

/* �C�x���g�̎����擾 */
long __stdcall MIDIEvent_GetTime (MIDIEvent* pEvent) {
	assert (pEvent);
	return pEvent->m_lTime;
}






/* �P�̃C�x���g�̎����ݒ� */
/* �C�x���g�����X�g�̗v�f�̏ꍇ�A�|�C���^���Ȃ��ς��Ď��������𐳂����ۂ��܂��B */
long __stdcall MIDIEvent_SetTimeSingle (MIDIEvent* pEvent, long lTime) {
	long lCurrentTime = pEvent->m_lTime;
	MIDITrack* pTrack = (MIDITrack*)(pEvent->m_pParent);
	assert (pEvent);

	/* ���V�C�x���g�̏ꍇ�͒P���Ɏ����ݒ� */
	if (MIDIEvent_IsFloating (pEvent)) {
		pEvent->m_lTime = CLIP (0, lTime, 0x7FFFFFFF);
		return 1;
	}

	/* �ȉ��͕��V�C�x���g�łȂ��ꍇ�̏��� */
	/* EOT�C�x���g�𓮂����ꍇ�̓��ꏈ�� */
	if (pEvent->m_lKind == MIDIEVENT_ENDOFTRACK && pEvent->m_pNextEvent == NULL) {
		/* EOT�C�x���g�̑O�ɕʂ̃C�x���g������ꍇ */
		if (pEvent->m_pPrevEvent) {
			/* EOT�C�x���g�͂��̃C�x���g���O�ɂ͈ړ����Ȃ��B */
			if (pEvent->m_pPrevEvent->m_lTime > lTime) {
				pEvent->m_lTime = pEvent->m_pPrevEvent->m_lTime;
			}
			else {
				pEvent->m_lTime = lTime;
			}
		}
		/* EOT�C�x���g�̑O�ɕʂ̃C�x���g�������ꍇ */
		else {
			/* �^�C���X�^���v0���O�ɂ͈ړ����Ȃ��B */
			pEvent->m_lTime = CLIP (0, lTime, 0x7FFFFFFF);
		}
		return 1;
	}

	/* �G���h�I�u�g���b�N�ȊO�̃C�x���g�̏ꍇ */
	/* ���݂̃^�C��������֓������ꍇ */
	if (lTime >= lCurrentTime) {
		/* pTempEvent�̒��O�ɑ}������BpTempEvent���Ȃ���΍Ō�ɑ}������B */
		MIDIEvent* pTempEvent = pEvent;
		MIDIEvent* pLastEvent = NULL;
		/* �m�[�g�I�t�C�x���g�̏ꍇ */
		if (MIDIEvent_IsNoteOff (pTempEvent)) {
			MIDIEvent* pNoteOnEvent = pTempEvent->m_pPrevCombinedEvent;
			/* �Ή�����m�[�g�I���C�x���g������ꍇ(20090713�ǉ�) */
			if (pNoteOnEvent) {
				/* ������=0�ȉ��̏ꍇ(20090713�ǉ�) */
				/* �Ή�����m�[�g�I���C�x���g�̒���Ɋm�� */
				if (lTime <= pNoteOnEvent->m_lTime) {
					lTime = pNoteOnEvent->m_lTime;
					pLastEvent = pNoteOnEvent;
					pTempEvent = pNoteOnEvent->m_pNextEvent;
					if (pTempEvent->m_lKind == MIDIEVENT_ENDOFTRACK &&
						pTempEvent->m_pNextEvent == NULL) {
						pTempEvent->m_lTime = lTime;
					}
				}
				/* ������=0�ȏ�̏ꍇ(20090713�ǉ�) */
				else {
					while (pTempEvent) {
						if (pTempEvent->m_lTime > lTime ||
							(pTempEvent->m_lTime == lTime && !MIDIEvent_IsNoteOff (pTempEvent))) {
							break;
						}
						/* EOT������ɗ���ꍇ��EOT�����֒ǂ����� */
						if (pTempEvent->m_lKind == MIDIEVENT_ENDOFTRACK &&
							pTempEvent->m_pNextEvent == NULL) {
							pTempEvent->m_lTime = lTime;
							break;
						}
						pLastEvent = pTempEvent;
						pTempEvent = pTempEvent->m_pNextEvent;
					}
				}
			}
			/* �Ή�����m�[�g�I���C�x���g���Ȃ��ꍇ */
			else {
				while (pTempEvent) {
					if (pTempEvent->m_lTime > lTime ||
						(pTempEvent->m_lTime == lTime && !MIDIEvent_IsNoteOff (pTempEvent))) {
						break;
					}
					/* EOT������ɗ���ꍇ��EOT�����֒ǂ����� */
					if (pTempEvent->m_lKind == MIDIEVENT_ENDOFTRACK &&
						pTempEvent->m_pNextEvent == NULL) {
						pTempEvent->m_lTime = lTime;
						break;
					}
					pLastEvent = pTempEvent;
					pTempEvent = pTempEvent->m_pNextEvent;
				}
			}
		}
		/* ���̑��̏ꍇ */
		else {
			while (pTempEvent) {
				if (pTempEvent->m_lTime > lTime) {
					break;
				}
				/* EOT������ɗ���ꍇ��EOT�����֒ǂ����� */
				if (pTempEvent->m_lKind == MIDIEVENT_ENDOFTRACK &&
					pTempEvent->m_pNextEvent == NULL) {
					pTempEvent->m_lTime = lTime;
					break;
				}
				pLastEvent = pTempEvent;
				pTempEvent = pTempEvent->m_pNextEvent;
			}
		}
		/* pTempEvent�̒��O��pEvent��}������ꍇ */
		if (pTempEvent) {
			/* if (pTempEvent != pEvent) { 20080622�p�~ */
			if (pTempEvent->m_pPrevEvent != pEvent) { /* 20080622�C�� */
				MIDIEvent_SetFloating (pEvent);
				pEvent->m_lTime = lTime;
				MIDIEvent_SetPrevEvent (pTempEvent, pEvent);
			}
			else {
				pEvent->m_lTime = lTime;
			}
		}
		/* �����N���X�g�̍Ō��pEvent��}������ꍇ */
		else if (pLastEvent) {
			/* if (pLastEvent != pEvent) { 20080622�p�~ */
			if (pLastEvent->m_pNextEvent != pEvent) { /* 20080622�C�� */
				MIDIEvent_SetFloating (pEvent);
				pEvent->m_lTime = lTime;
				MIDIEvent_SetNextEvent (pLastEvent, pEvent);
			}
			else {
				pEvent->m_lTime = lTime;
			}
		}
		/* ��̃��X�g�ɑ}������ꍇ */
		else if (pTrack) {
			pEvent->m_lTime = lTime;
			pEvent->m_pParent = pTrack;
			pEvent->m_pNextEvent = NULL;
			pEvent->m_pPrevEvent = NULL;
			pEvent->m_pNextSameKindEvent = NULL;
			pEvent->m_pPrevSameKindEvent = NULL;
			pTrack->m_pFirstEvent = pEvent;
			pTrack->m_pLastEvent = pEvent;
			pTrack->m_lNumEvent ++;
		}

	}
	/* ���݂̃^�C�����O���֓������ꍇ */
	else if (lTime < lCurrentTime) {
		/* pTempEvent�̒���ɑ}������BpTempEvent���Ȃ���΍ŏ��ɑ}������B */
		MIDIEvent* pTempEvent = pEvent;
		MIDIEvent* pFirstEvent = NULL;
		/* �m�[�g�I�t�C�x���g�̏ꍇ */
		if (MIDIEvent_IsNoteOff (pEvent)) {
			MIDIEvent* pNoteOnEvent = pTempEvent->m_pPrevCombinedEvent;
			/* �Ή�����m�[�g�I���C�x���g������ꍇ(20090713�ǉ�) */
			if (pNoteOnEvent) {
				/* ������=0�ȉ��̏ꍇ(20090713�ǉ�) */
				/* �Ή�����m�[�g�I���C�x���g�̒���Ɋm�� */
				if (lTime <= pNoteOnEvent->m_lTime) {
					lTime = pNoteOnEvent->m_lTime;
					pFirstEvent = NULL;
					pTempEvent = pNoteOnEvent;
				}
				/* ������=0�ȏ�̏ꍇ(20090713�ǉ�) */
				else {
					while (pTempEvent) {
						if (pTempEvent->m_lTime < lTime ||
							(pTempEvent->m_lTime == lTime && MIDIEvent_IsNoteOff (pTempEvent))) {
							break;
						}
						/* �Ή�����m�[�g�I���C�x���g���O�ɂ͍s���Ȃ� */
						if (pTempEvent == pNoteOnEvent) {
							break;
						}
						pFirstEvent = pTempEvent;
						pTempEvent = pTempEvent->m_pPrevEvent;
					}
				}
			}
			/* �Ή�����m�[�g�I���C�x���g���Ȃ��ꍇ */
			else {
				while (pTempEvent) {
					if (pTempEvent->m_lTime < lTime ||
						(pTempEvent->m_lTime == lTime && MIDIEvent_IsNoteOff (pTempEvent))) {
						break;
					}
					pFirstEvent = pTempEvent;
					pTempEvent = pTempEvent->m_pPrevEvent;
				}
			}
		}
		/* ���̑��̃C�x���g�̏ꍇ */
		else {
			while (pTempEvent) {
				if (pTempEvent->m_lTime <= lTime) {
					break;
				}
				pFirstEvent = pTempEvent;
				pTempEvent = pTempEvent->m_pPrevEvent;
			}
		}
		/* pTempEvent�̒����pEvent��}������ꍇ */
		if (pTempEvent) {
			if (pTempEvent != pEvent && 
				pTempEvent->m_pNextEvent != pEvent) { /* 20080721�C�� */
				MIDIEvent_SetFloating (pEvent);
				pEvent->m_lTime = lTime;
				MIDIEvent_SetNextEvent (pTempEvent, pEvent);
			}
			else {
				pEvent->m_lTime = lTime;
			}
		}
		/* �����N���X�g�̍ŏ���pEvent��}������ꍇ */
		else if (pFirstEvent) {
			if (pFirstEvent != pEvent &&
				pFirstEvent->m_pPrevEvent != pEvent) { /* 20080721�ǉ� */
				MIDIEvent_SetFloating (pEvent);
				pEvent->m_lTime = lTime;
				MIDIEvent_SetPrevEvent (pFirstEvent, pEvent);
			}
			else {
				pEvent->m_lTime = lTime;
			}
		}
		/* ��̃��X�g�ɑ}������ꍇ */
		else if (pTrack) {
			pEvent->m_lTime = lTime;
			pEvent->m_pParent = pTrack;
			pEvent->m_pNextEvent = NULL;
			pEvent->m_pPrevEvent = NULL;
			pEvent->m_pNextSameKindEvent = NULL;
			pEvent->m_pPrevSameKindEvent = NULL;
			pTrack->m_pFirstEvent = pEvent;
			pTrack->m_pLastEvent = pEvent;
			pTrack->m_lNumEvent ++;
		}
	}
	return 1;
}

/* �C�x���g�̎����ݒ� */
/* (�����C�x���g���A�����Ď��������ΓI�ɓ����܂�) */
long __stdcall MIDIEvent_SetTime (MIDIEvent* pEvent, long lTime) {
	long nCounter = 0;
	long lTargetTime, lDeltaTime;
	MIDIEvent* pMoveEvent = NULL;
	lTime = CLIP (0, lTime, 0x7FFFFFFF);
	lDeltaTime = lTime - pEvent->m_lTime;
	pMoveEvent = MIDIEvent_GetFirstCombinedEvent (pEvent);
	while (pMoveEvent) {
		lTargetTime = pMoveEvent->m_lTime + lDeltaTime;
		lTargetTime = CLIP (0, lTargetTime, 0x7FFFFFFF);
		MIDIEvent_SetTimeSingle (pMoveEvent, lTargetTime);
		nCounter++;
		pMoveEvent = pMoveEvent->m_pNextCombinedEvent;
	}
	return nCounter;
}

/* �C�x���g�̃L�[�擾(�m�[�g�I�t�E�m�[�g�I���E�`�����l���A�t�^�[�̂�) */
long __stdcall MIDIEvent_GetKey (MIDIEvent* pEvent) {
	assert (pEvent);
	assert (0x80 <= pEvent->m_lKind && pEvent->m_lKind <= 0xAF);
	if (0x80 <= pEvent->m_lKind && pEvent->m_lKind <= 0xAF) {
		return *(pEvent->m_pData + 1);
	}
	return 0;
}

/* �C�x���g�̃L�[�ݒ�(�m�[�g�I�t�E�m�[�g�I���E�`�����l���A�t�^�[�̂�) */
long __stdcall MIDIEvent_SetKey (MIDIEvent* pEvent, long lKey) {
	long lCount = 0;
	MIDIEvent* pTempEvent = pEvent;
	assert (pEvent);
	assert (0x80 <= pEvent->m_lKind && pEvent->m_lKind <= 0xAF);
	pTempEvent = MIDIEvent_GetFirstCombinedEvent (pEvent);
	while (pTempEvent) {
		if (0x80 <= pTempEvent->m_lKind && pTempEvent->m_lKind <= 0xAF) {
			*(pTempEvent->m_pData + 1) = (unsigned char)(CLIP (0, lKey, 127));
			lCount++;
		}
		pTempEvent = pTempEvent->m_pNextCombinedEvent;
	}
	return lCount;
}

/* �C�x���g�̃x���V�e�B�擾(�m�[�g�I�t�E�m�[�g�I���̂�) */
long __stdcall MIDIEvent_GetVelocity (MIDIEvent* pEvent) {
	assert (pEvent);
	assert (0x80 <= pEvent->m_lKind && pEvent->m_lKind <= 0x9F);
	if (0x80 <= pEvent->m_lKind && pEvent->m_lKind <= 0x9F) {
		return *(pEvent->m_pData + 2);
	}
	return 0;
}

/* �C�x���g�̃x���V�e�B�ݒ�(�m�[�g�I�t�E�m�[�g�I���̂�) */
long __stdcall MIDIEvent_SetVelocity (MIDIEvent* pEvent, long lVel) {
	assert (pEvent);
	assert (0x80 <= pEvent->m_lKind && pEvent->m_lKind <= 0x9F);
	if (0x80 <= pEvent->m_lKind && pEvent->m_lKind <= 0x8F) {
		*(pEvent->m_pData + 2) = (unsigned char)(CLIP (0, lVel, 127));
		return 1;
	}
	else if (0x90 <= pEvent->m_lKind && pEvent->m_lKind <= 0x9F) {
		if (*(pEvent->m_pData + 2) >= 1) {
			*(pEvent->m_pData + 2) = (unsigned char)(CLIP (1, lVel, 127));
			return 1;
		}
		return 0;
	}
	return 0;
}

/* �����C�x���g�̉������擾(�m�[�g�̂�) */
long __stdcall MIDIEvent_GetDuration (MIDIEvent* pEvent) {
	long lDuration = 0;
	MIDIEvent* pNoteOnEvent = NULL;
	MIDIEvent* pNoteOffEvent = NULL;
	assert (pEvent);
	assert (0x80 <= pEvent->m_lKind && pEvent->m_lKind <= 0x9F);
	if (!MIDIEvent_IsNote (pEvent)) {
		return 0;
	}
	if (MIDIEvent_IsNoteOn (pEvent)) {
		pNoteOnEvent = pEvent;
		pNoteOffEvent = pEvent->m_pNextCombinedEvent;
		lDuration = pNoteOffEvent->m_lTime - pNoteOnEvent->m_lTime;
		assert (lDuration >= 0);
	}
	else if (MIDIEvent_IsNoteOff (pEvent)) {
		pNoteOffEvent = pEvent;
		pNoteOnEvent = pEvent->m_pPrevCombinedEvent;
		lDuration = pNoteOnEvent->m_lTime - pNoteOffEvent->m_lTime;
		assert (lDuration <= 0);
	}
	return lDuration;

}

/* �����C�x���g�̉������ݒ�(�m�[�g�̂�) */
long __stdcall MIDIEvent_SetDuration (MIDIEvent* pEvent, long lDuration) {
	long lTime = 0;
	MIDIEvent* pNoteOnEvent = NULL;
	MIDIEvent* pNoteOffEvent = NULL;
	assert (pEvent);
	assert (0x80 <= pEvent->m_lKind && pEvent->m_lKind <= 0x9F);
	if (!MIDIEvent_IsNote (pEvent)) {
		return 0;
	}
	if (MIDIEvent_IsNoteOn (pEvent)) {
		if (lDuration < 0) {
			return 0;
		}
		pNoteOnEvent = pEvent;
		pNoteOffEvent = pEvent->m_pNextCombinedEvent;
		lTime = CLIP (0, pNoteOnEvent->m_lTime + lDuration, 0x7FFFFFFF);
		return MIDIEvent_SetTimeSingle (pNoteOffEvent, lTime);
	}
	else if (MIDIEvent_IsNoteOff (pEvent)) {
		if (lDuration > 0) {
			return 0;
		}
		pNoteOffEvent = pEvent;
		pNoteOnEvent = pEvent->m_pPrevCombinedEvent;
		lTime = CLIP (0, pNoteOffEvent->m_lTime + lDuration, 0x7FFFFFFF);
		/* TODO:lDuration==0�̂Ƃ��ANoteOn�̂ق�����ɗ��Ă��܂��B*/
		return MIDIEvent_SetTimeSingle (pNoteOnEvent, lTime);
	}
	return 1;
}


/* �����C�x���g�̃o���N�擾(RPN�`�F���W�ENRPN�`�F���W�E�p�b�`�`�F���W�̂�) */
long __stdcall MIDIEvent_GetBank (MIDIEvent* pEvent) {
	long lBankMSB = 0; /* �o���N���:CC#0,CC#99,CC#101 */
	long lBankLSB = 0; /* �o���N����:CC#32,CC#98,CC#100 */
	MIDIEvent* pMSBEvent = NULL;
	MIDIEvent* pLSBEvent = NULL;
	assert (pEvent);
	assert (0xB0 <= pEvent->m_lKind && pEvent->m_lKind <= 0xCF);
	if (MIDIEvent_IsPatchChange (pEvent) ||
		MIDIEvent_IsRPNChange (pEvent) || MIDIEvent_IsNRPNChange (pEvent)) {
		pMSBEvent = MIDIEvent_GetFirstCombinedEvent (pEvent);
		assert (MIDIEvent_IsControlChange (pMSBEvent));
		lBankMSB = MIDIEvent_GetValue (pMSBEvent);
		pLSBEvent = pMSBEvent->m_pNextCombinedEvent;
		assert (MIDIEvent_IsControlChange (pLSBEvent));
		lBankLSB = MIDIEvent_GetValue (pLSBEvent);
		return (lBankMSB << 7) | (lBankLSB & 0x7F);
	}
	else if (MIDIEvent_IsProgramChange (pEvent)) {
		long lBankMSBFound = 0;
		long lBankLSBFound = 0;
		long lChannel = MIDIEvent_GetChannel (pEvent);
		MIDIEvent* pTempEvent = pEvent->m_pPrevEvent;		
		while (pTempEvent) {
			if (MIDIEvent_IsControlChange (pTempEvent)) {
				if (MIDIEvent_GetChannel (pTempEvent) == lChannel) {
					if (MIDIEvent_GetNumber (pTempEvent) == 0) {
						lBankMSB = MIDIEvent_GetValue (pTempEvent);
						lBankMSBFound++;
					}
					else if (MIDIEvent_GetNumber (pTempEvent) == 32) {
						lBankLSB = MIDIEvent_GetValue (pTempEvent);
						lBankLSBFound++;
					}
					if (lBankMSBFound && lBankLSBFound) {
						break;
					}
				}
			}
			pTempEvent = pTempEvent->m_pPrevEvent;
		}
		return (lBankMSB << 7) | (lBankLSB & 0x7F);
	}
	else if (MIDIEvent_IsControlChange (pEvent)) {
		long lNumber = MIDIEvent_GetNumber (pEvent);
		if (lNumber == 6 || lNumber == 38) {
			long lBankMSBFound = 0;
			long lBankLSBFound = 0;
			long lChannel = MIDIEvent_GetChannel (pEvent);
			MIDIEvent* pTempEvent = pEvent->m_pPrevEvent;
			while (pTempEvent) {
				if (MIDIEvent_IsControlChange (pTempEvent)) {
					if (MIDIEvent_GetChannel (pTempEvent) == lChannel) {
						if (MIDIEvent_GetNumber (pTempEvent) == 99 ||
							MIDIEvent_GetNumber (pTempEvent) == 101) {
							lBankMSB = MIDIEvent_GetValue (pTempEvent);
							lBankMSBFound++;
						}
						else if (MIDIEvent_GetNumber (pTempEvent) == 98 ||
							MIDIEvent_GetNumber (pTempEvent) == 100) {
							lBankLSB = MIDIEvent_GetValue (pTempEvent);
							lBankLSBFound++;
						}
						if (lBankMSBFound && lBankLSBFound) {
							break;
						}
					}
				}
				pTempEvent = pTempEvent->m_pPrevEvent;
			}
		}
		return (lBankMSB << 7) | (lBankLSB & 0x7F);
	}
	return 0;
}

/* �����C�x���g�̃o���N���(MSB)�擾(RPN�`�F���W�ENRPN�`�F���W�E�p�b�`�`�F���W�̂�) */
long __stdcall MIDIEvent_GetBankMSB (MIDIEvent* pEvent) {
	long lBankMSB = 0; /* �o���N���:CC#0,CC#99,CC#101 */
	long lBankLSB = 0; /* �o���N����:CC#32,CC#98,CC#100 */
	MIDIEvent* pMSBEvent = NULL;
	MIDIEvent* pLSBEvent = NULL;
	assert (pEvent);
	assert (0xB0 <= pEvent->m_lKind && pEvent->m_lKind <= 0xCF);
	if (MIDIEvent_IsPatchChange (pEvent) ||
		MIDIEvent_IsRPNChange (pEvent) || MIDIEvent_IsNRPNChange (pEvent)) {
		pMSBEvent = MIDIEvent_GetFirstCombinedEvent (pEvent);
		assert (MIDIEvent_IsControlChange (pMSBEvent));
		lBankMSB = MIDIEvent_GetValue (pMSBEvent);
		pLSBEvent = pMSBEvent->m_pNextCombinedEvent;
		assert (MIDIEvent_IsControlChange (pLSBEvent));
		lBankLSB = MIDIEvent_GetValue (pLSBEvent);
		return lBankMSB;
	}
	else if (MIDIEvent_IsProgramChange (pEvent)) {
		long lChannel = MIDIEvent_GetChannel (pEvent);
		MIDIEvent* pTempEvent = pEvent->m_pPrevEvent;
		while (pTempEvent) {
			if (MIDIEvent_IsControlChange (pTempEvent)) {
				if (MIDIEvent_GetChannel (pTempEvent) == lChannel) {
					if (MIDIEvent_GetNumber (pTempEvent) == 0) {
						return MIDIEvent_GetValue (pTempEvent);
					}
				}
			}
			pTempEvent = pTempEvent->m_pPrevEvent;
		}
		return 0;
	}
	else if (MIDIEvent_IsControlChange (pEvent)) {
		long lNumber = MIDIEvent_GetNumber (pEvent);
		if (lNumber == 6 || lNumber == 38) {
			long lChannel = MIDIEvent_GetChannel (pEvent);
			MIDIEvent* pTempEvent = pEvent->m_pPrevEvent;
			while (pTempEvent) {
				if (MIDIEvent_IsControlChange (pTempEvent)) {
					if (MIDIEvent_GetChannel (pTempEvent) == lChannel) {
						if (MIDIEvent_GetNumber (pTempEvent) == 99 ||
							MIDIEvent_GetNumber (pTempEvent) == 101) {
							return MIDIEvent_GetValue (pTempEvent);
						}
					}
				}
				pTempEvent = pTempEvent->m_pPrevEvent;
			}
		}
		return 0;
	}
	return 0;
}

/* �����C�x���g�̃o���N����(LSB)�擾(RPN�`�F���W�ENRPN�`�F���W�E�p�b�`�`�F���W�̂�) */
long __stdcall MIDIEvent_GetBankLSB (MIDIEvent* pEvent) {
	long lBankMSB = 0; /* �o���N���:CC#0,CC#99,CC#101 */
	long lBankLSB = 0; /* �o���N����:CC#32,CC#98,CC#100 */
	MIDIEvent* pMSBEvent = NULL;
	MIDIEvent* pLSBEvent = NULL;
	assert (pEvent);
	assert (0xB0 <= pEvent->m_lKind && pEvent->m_lKind <= 0xCF);
	if (MIDIEvent_IsPatchChange (pEvent) ||
		MIDIEvent_IsRPNChange (pEvent) || MIDIEvent_IsNRPNChange (pEvent)) {
		pMSBEvent = MIDIEvent_GetFirstCombinedEvent (pEvent);
		assert (MIDIEvent_IsControlChange (pMSBEvent));
		lBankMSB = MIDIEvent_GetValue (pMSBEvent);
		pLSBEvent = pMSBEvent->m_pNextCombinedEvent;
		assert (MIDIEvent_IsControlChange (pLSBEvent));
		lBankLSB = MIDIEvent_GetValue (pLSBEvent);
		return lBankLSB;
	}
	else if (MIDIEvent_IsProgramChange (pEvent)) {
		long lChannel = MIDIEvent_GetChannel (pEvent);
		MIDIEvent* pTempEvent = pEvent->m_pPrevEvent;
		while (pTempEvent) {
			if (MIDIEvent_IsControlChange (pTempEvent)) {
				if (MIDIEvent_GetChannel (pTempEvent) == lChannel) {
					if (MIDIEvent_GetNumber (pTempEvent) == 32) {
						return MIDIEvent_GetValue (pTempEvent);
					}
				}
			}
			pTempEvent = pTempEvent->m_pPrevEvent;
		}
		return 0;
	}
	else if (MIDIEvent_IsControlChange (pEvent)) {
		long lNumber = MIDIEvent_GetNumber (pEvent);
		if (lNumber == 6 || lNumber == 38) {
			long lChannel = MIDIEvent_GetChannel (pEvent);
			MIDIEvent* pTempEvent = pEvent->m_pPrevEvent;
			while (pTempEvent) {
				if (MIDIEvent_IsControlChange (pTempEvent)) {
					if (MIDIEvent_GetChannel (pTempEvent) == lChannel) {
						if (MIDIEvent_GetNumber (pTempEvent) == 98 ||
							MIDIEvent_GetNumber (pTempEvent) == 100) {
							return MIDIEvent_GetValue (pTempEvent);
						}
					}
				}
				pTempEvent = pTempEvent->m_pPrevEvent;
			}
		}
		return 0;
	}
	return 0;
}

/* �����C�x���g�̃o���N�ݒ�(RPN�`�F���W�ENRPN�`�F���W�E�p�b�`�`�F���W�̂�) */
long __stdcall MIDIEvent_SetBank (MIDIEvent* pEvent, long lBank) {
	long lBankMSB = (CLIP(0, lBank, 16383) >> 7); /* �o���N���:CC#0,CC#99,CC#101 */
	long lBankLSB = (CLIP(0, lBank, 16383) & 0x7F); /* �o���N����:CC#32,CC#98,CC#100 */
	MIDIEvent* pMSBEvent = NULL;
	MIDIEvent* pLSBEvent = NULL;
	assert (pEvent);
	assert (0xB0 <= pEvent->m_lKind && pEvent->m_lKind <= 0xCF);
	if (MIDIEvent_IsPatchChange (pEvent) ||
		MIDIEvent_IsRPNChange (pEvent) || MIDIEvent_IsNRPNChange (pEvent)) {
		pMSBEvent = MIDIEvent_GetFirstCombinedEvent (pEvent);
		assert (MIDIEvent_IsControlChange (pMSBEvent));
		MIDIEvent_SetValue (pMSBEvent, lBankMSB);
		pLSBEvent = pMSBEvent->m_pNextCombinedEvent;
		assert (MIDIEvent_IsControlChange (pLSBEvent));
		MIDIEvent_SetValue (pLSBEvent, lBankLSB);
		return 2;
	}
	return 0;
}

/* �����C�x���g�̃o���N���(MSB)�ݒ�(RPN�`�F���W�ENRPN�`�F���W�E�p�b�`�`�F���W�̂�) */
long __stdcall MIDIEvent_SetBankMSB (MIDIEvent* pEvent, long lBankMSB) {
	MIDIEvent* pMSBEvent = NULL;
	assert (pEvent);
	assert (0xB0 <= pEvent->m_lKind && pEvent->m_lKind <= 0xCF);
	if (MIDIEvent_IsPatchChange (pEvent) ||
		MIDIEvent_IsRPNChange (pEvent) || MIDIEvent_IsNRPNChange (pEvent)) {
		pMSBEvent = MIDIEvent_GetFirstCombinedEvent (pEvent);
		assert (MIDIEvent_IsControlChange (pMSBEvent));
		return 1;
	}
	return 0;
}

/* �����C�x���g�̃o���N����(LSB)�ݒ�(RPN�`�F���W�ENRPN�`�F���W�E�p�b�`�`�F���W�̂�) */
long __stdcall MIDIEvent_SetBankLSB (MIDIEvent* pEvent, long lBankLSB) {
	MIDIEvent* pMSBEvent = NULL;
	MIDIEvent* pLSBEvent = NULL;
	assert (pEvent);
	assert (0xB0 <= pEvent->m_lKind && pEvent->m_lKind <= 0xCF);
	if (MIDIEvent_IsPatchChange (pEvent) ||
		MIDIEvent_IsRPNChange (pEvent) || MIDIEvent_IsNRPNChange (pEvent)) {
		pMSBEvent = MIDIEvent_GetFirstCombinedEvent (pEvent);
		pLSBEvent = pMSBEvent->m_pNextCombinedEvent;
		assert (MIDIEvent_IsControlChange (pLSBEvent));
		MIDIEvent_SetValue (pLSBEvent, lBankLSB);
		return 1;
	}
	return 0;
}

/* �C�x���g�̔ԍ��擾(�V�[�P���X�ԍ��E�`�����l���v���t�B�b�N�X�E�|�[�g�v���t�B�b�N�X�E */
/* �R���g���[���`�F���W�E�v���O�����`�F���W) */
long __stdcall MIDIEvent_GetNumber (MIDIEvent* pEvent) {
	assert (pEvent);
	/* �V�[�P���X�ԍ��̏ꍇ */
	if (MIDIEvent_IsSequenceNumber (pEvent)) {
		return (*(pEvent->m_pData) << 8) + *(pEvent->m_pData + 1);
	}
	/* �`�����l���v���t�B�b�N�X�A�|�[�g�v���t�B�b�N�X�̏ꍇ */
	else if (MIDIEvent_IsChannelPrefix (pEvent) || MIDIEvent_IsPortPrefix (pEvent)) {
		return *(pEvent->m_pData);
	}
	/* �R���g���[���`�F���W�̏ꍇ */
	else if (MIDIEvent_IsControlChange (pEvent)) {
		return *(pEvent->m_pData + 1);
	}
	/* �v���O�����`�F���W */
	else if (MIDIEvent_IsProgramChange (pEvent)) {
		return *(pEvent->m_pData + 1);
	}
	return 0;
}

/* �C�x���g�̔ԍ��ݒ�(�V�[�P���X�ԍ��E�`�����l���v���t�B�b�N�X�E�|�[�g�v���t�B�b�N�X�E */
/* �R���g���[���`�F���W�E�v���O�����`�F���W) */
long __stdcall MIDIEvent_SetNumber (MIDIEvent* pEvent, long lNum) {
	assert (pEvent);
	/* �V�[�P���X�ԍ��̏ꍇ */
	if (MIDIEvent_IsSequenceNumber (pEvent)) {
		*(pEvent->m_pData + 0) = (unsigned char)(CLIP (0, lNum, 65535) >> 8);
		*(pEvent->m_pData + 1) = (unsigned char)(CLIP (0, lNum, 65535) & 0xFF);
		return 1;
	}
	/* �`�����l���v���t�B�b�N�X�̏ꍇ */
	else if (MIDIEvent_IsChannelPrefix (pEvent)) {
		*(pEvent->m_pData + 0) = (unsigned char)(CLIP (0, lNum, 15));
		return 1;
	}
	/* �|�[�g�v���t�B�b�N�X�̏ꍇ */
	else if (MIDIEvent_IsPortPrefix (pEvent)) {
		*(pEvent->m_pData + 0) = (unsigned char)(CLIP (0, lNum, 255));
		return 1;
	}
	/* �R���g���[���`�F���W�̏ꍇ */
	else if (MIDIEvent_IsControlChange (pEvent)) {
		/* RPN�`�F���W�ENRPN�`�F���W�E�p�b�`�`�F���W�̏ꍇ�ACC#��؂�ւ���̂͋֎~ */
		if (MIDIEvent_IsRPNChange (pEvent) || MIDIEvent_IsNRPNChange (pEvent) ||
			MIDIEvent_IsPatchChange (pEvent)) {
			return 0;
		}
		/* ���̑��̏ꍇ��CC#��؂�ւ����� */
		else {
			*(pEvent->m_pData + 1) = (unsigned char)(CLIP (0, lNum, 127));
			return 1;
		}
	}
	/* �v���O�����`�F���W�̏ꍇ */
	else if (MIDIEvent_IsProgramChange (pEvent)) {
		*(pEvent->m_pData + 1) = (unsigned char)(CLIP (0, lNum, 127));
		return 1;
	}
	return 0;
}

/* �C�x���g�̒l�擾(�V�[�P���X�ԍ��E�`�����l���v���t�B�b�N�X�E�|�[�g�v���t�B�b�N�X�E */
/* �L�[�A�t�^�[�E�R���g���[���`�F���W�E�v���O�����`�F���W�E�`�����l���A�t�^�[�E�s�b�`�x���h) */
long __stdcall MIDIEvent_GetValue (MIDIEvent* pEvent) {
	assert (pEvent);
	/* �V�[�P���X�ԍ��̏ꍇ */
	if (MIDIEvent_IsSequenceNumber (pEvent)) {
		return (*(pEvent->m_pData) << 8) + *(pEvent->m_pData + 1);
	}
	/* �`�����l���v���t�B�b�N�X�E�|�[�g�v���t�B�b�N�X�̏ꍇ */
	else if (MIDIEvent_IsChannelPrefix (pEvent) || MIDIEvent_IsPortPrefix (pEvent)) {
		return *(pEvent->m_pData);
	}
	/* �L�[�A�t�^�[�^�b�`�E�R���g���[���`�F���W�̏ꍇ */
	else if (MIDIEvent_IsKeyAftertouch (pEvent) || MIDIEvent_IsControlChange (pEvent)) {
		return *(pEvent->m_pData + 2);
	}
	/* �v���O�����`�F���W�E�`�����l���A�t�^�[�^�b�`�̏ꍇ */
	else if (MIDIEvent_IsProgramChange (pEvent) || MIDIEvent_IsChannelAftertouch (pEvent)) {
		return *(pEvent->m_pData + 1);
	}
	/* �s�b�`�x���h�̏ꍇ */
	else if (MIDIEvent_IsPitchBend (pEvent)) {
		return *(pEvent->m_pData + 1) + (*(pEvent->m_pData + 2) << 7);
	}
	return 0;
}

/* �C�x���g�̒l�ݒ�(�V�[�P���X�ԍ��E�`�����l���v���t�B�b�N�X�E�|�[�g�v���t�B�b�N�X�E */
/* �L�[�A�t�^�[�E�R���g���[���`�F���W�E�v���O�����`�F���W�E�`�����l���A�t�^�[�E�s�b�`�x���h) */
long __stdcall MIDIEvent_SetValue (MIDIEvent* pEvent, long lVal) {
	assert (pEvent);
	/* �V�[�P���X�ԍ��̏ꍇ */
	if (MIDIEvent_IsSequenceNumber (pEvent)) {
		*(pEvent->m_pData + 0) = (unsigned char)(CLIP (0, lVal, 65535) >> 8);
		*(pEvent->m_pData + 1) = (unsigned char)(CLIP (0, lVal, 65535) & 0x00FF);
		return 1;
	}
	/* �`�����l���v���t�B�b�N�X */
	else if (MIDIEvent_IsChannelPrefix (pEvent)) {
		*(pEvent->m_pData + 0) = (unsigned char)(CLIP (0, lVal, 15));
		return 1;
	}
	/* �|�[�g�v���t�B�b�N�X�̏ꍇ */
	else if (MIDIEvent_IsPortPrefix (pEvent)) {
		*(pEvent->m_pData + 0) = (unsigned char)(CLIP (0, lVal, 255));
		return 1;
	}
	/* �L�[�A�t�^�[�^�b�`�E�R���g���[���`�F���W�̏ꍇ */
	else if (MIDIEvent_IsKeyAftertouch (pEvent) || MIDIEvent_IsControlChange (pEvent)) {
		*(pEvent->m_pData + 2) = (unsigned char)(CLIP (0, lVal, 127));
		return 1;
	}
	/* �v���O�����`�F���W�E�`�����l���A�t�^�[�^�b�`�̏ꍇ */
	else if (MIDIEvent_IsProgramChange (pEvent) || MIDIEvent_IsChannelAftertouch (pEvent)) {
		*(pEvent->m_pData + 1) = (unsigned char)(CLIP (0, lVal, 127));
		return 1;
	}
	/* �s�b�`�x���h�̏ꍇ */
	else if (MIDIEvent_IsPitchBend (pEvent)) {
		*(pEvent->m_pData + 1) = (unsigned char)(CLIP (0, lVal, 16383) & 0x007F);
		*(pEvent->m_pData + 2) = (unsigned char)(CLIP (0, lVal, 16383) >> 7);
		return 1;
	}
	return 0;
}

/* ���̃C�x���g�擾(�Ȃ����NULL) */
MIDIEvent* __stdcall MIDIEvent_GetNextEvent (MIDIEvent* pEvent) {
	return (pEvent->m_pNextEvent);
}

/* �O�̃C�x���g�擾(�Ȃ����NULL) */
MIDIEvent* __stdcall MIDIEvent_GetPrevEvent (MIDIEvent* pEvent) {
	return (pEvent->m_pPrevEvent);
}

/* ���̓�����ނ̃C�x���g�擾(�Ȃ����NULL) */
MIDIEvent* __stdcall MIDIEvent_GetNextSameKindEvent (MIDIEvent* pEvent) {
	return (pEvent->m_pNextSameKindEvent);
}

/* �O�̓�����ނ̃C�x���g�擾(�Ȃ����NULL) */
MIDIEvent* __stdcall MIDIEvent_GetPrevSameKindEvent (MIDIEvent* pEvent) {
	return (pEvent->m_pPrevSameKindEvent);
}

/* �e�g���b�N�ւ̃|�C���^���擾(�Ȃ����NULL) */
MIDITrack* __stdcall MIDIEvent_GetParent (MIDIEvent* pMIDIEvent) {
	return (MIDITrack*)(pMIDIEvent->m_pParent);
}










/* �C�x���g�̎�ޕ�����\���\(���^�C�x���g) */
static char* g_szMetaKindName[] = {
	"SequenceNumber", "TextEvent", "CopyrightNotice", "TrackName",
	"InstrumentName", "Lyric", "Marker", "CuePoint",
	"ProgramName", "DeviceName", "", "", "", "", "", "", /* 0x00 �` 0x0F */
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", /* 0x10 �` 0x1F */
	"ChannelPrefix", "PortPrefix", "", "", "", "", "", "", 
	"", "", "", "", "", "", "", "EndofTrack", /* 0x20 �` 0x2F */
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", /* 0x30 �` 0x3F */
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", /* 0x40 �` 0x4F */
    "", "Tempo", "", "", "SMPTEOffset", "", "", "",
	"TimeSignature", "KeySignature", "", "", "", "", "", "", /* 0x50 �` 0x5F */
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", /* 0x60 �` 0x6F */
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "SequencerSpec", /* 0x70 �` 0x7F */
	"UnknownMetaEvent"};

/* �C�x���g�̎�ޕ�����\���\(MIDI�C�x���g) */
static char* g_szMIDIKindName[] = {
	"NoteOff", "NoteOn", "KeyAftertouch", "ControlChange",
	"ProgramChange", "ChannelAftertouch", "PitchBend"};

/* �C�x���g�̎�ޕ�����\���\(SYSEX�C�x���g) */
static char* g_szSysExKindName [] = {
	"SysExStart", "", "", "", "", "", "", "SysExContinue"
};

/* �m�[�g�L�[������\���\(���쒆) */
static char* g_szKeyName[] =
		{"C_", "C#", "D_", "D#", "E_", "F_", "F#", "G_", "G#", "A_", "Bb", "B_", /* �n(�}0) */
		 "C_", "Db", "D_", "Eb", "E_", "F_", "Gb", "G_", "Ab", "A_", "Bb", "Cb", /* �σj */
		 "C_", "C#", "D_", "D#", "E_", "F_", "F#", "G_", "G#", "A_", "Bb", "B_", /* �j(2#) */
		 "C_", "Db", "D_", "Eb", "E_", "F_", "Gb", "G_", "Ab", "A_", "Bb", "B_", /* �σz(3b) */
		 "C_", "C#", "D_", "D#", "E_", "E#", "F#", "G_" };		

/* MIDI�C�x���g�̓��e�𕶎���\���ɕϊ� */
char* __stdcall MIDIEvent_ToString (MIDIEvent* pEvent, char* pBuf, long lLen) {
	return MIDIEvent_ToStringEx (pEvent, pBuf, lLen, MIDIEVENT_DUMPALL);
}

/* MIDI�C�x���g�̓��e�𕶎���\���ɕϊ� */
char* __stdcall MIDIEvent_ToStringEx (MIDIEvent* pEvent, char* pBuf, long lLen, long lFlags) {
	unsigned char szText[2048];
	long lColumn = 0;
	long lOutput = 0;
	memset (szText, 0, 2048);
	memset (pBuf, 0, lLen);
	/* �����̏o�� */
	if (lFlags & MIDIEVENT_DUMPTIME) {
		MIDITrack* pMIDITrack;
		MIDIData* pMIDIData;
		long lMeasure, lBeat, lTick;
		if (lColumn != 0) {
			lOutput += sprintf (pBuf + lOutput, " ");
		}
		lColumn++;
		if ((pMIDITrack = (MIDITrack*)(pEvent->m_pParent))) {
			if ((pMIDIData = (MIDIData*)(pMIDITrack->m_pParent))) {
				long lMode = MIDIData_GetTimeMode (pMIDIData);
				long lResolution = MIDIData_GetTimeResolution (pMIDIData);
				if (lMode == MIDIDATA_TPQNBASE) {
					MIDITrack* pSetupTrack = (pMIDIData->m_lFormat == MIDIDATA_FORMAT2) ?
						pMIDITrack : pMIDIData->m_pFirstTrack;
					MIDITrack_BreakTime (pSetupTrack, pEvent->m_lTime, &lMeasure, &lBeat, &lTick);
					lOutput += sprintf (pBuf + lOutput, "%05ld:%02ld:%03ld", lMeasure, lBeat, lTick);
				}
				else {
					lOutput += sprintf (pBuf + lOutput, "%08ld:%03ld", 
						pEvent->m_lTime / lResolution, pEvent->m_lTime % lResolution);
				}
			}
			else {
				lOutput += sprintf (pBuf + lOutput, "%12ld", pEvent->m_lTime);
			}
		}
		else {
			lOutput += sprintf (pBuf + lOutput, "%12ld", pEvent->m_lTime);
		}
		//lOutput += sprintf (pBuf + lOutput, "%08d", pEvent->m_lTime);
	}
	/* ��ނ̏o�� */
	if (lFlags & MIDIEVENT_DUMPKIND) {
		if (lColumn != 0) {
			lOutput += sprintf (pBuf + lOutput, " ");
		}
		lColumn++;
		if (MIDIEvent_IsMetaEvent (pEvent)) {
			if (strcmp (g_szMetaKindName[pEvent->m_lKind], "") != 0) {
				lOutput += sprintf (pBuf + lOutput, "%-17.17s", g_szMetaKindName[pEvent->m_lKind]);
			}
			else {
				lOutput += sprintf (pBuf + lOutput, "0x%02lX             ", pEvent->m_lKind);
			}
		}
		else if (MIDIEvent_IsMIDIEvent (pEvent)) {
			long lTempKind = pEvent->m_lKind;
			if (MIDIEvent_IsNoteOff (pEvent)) {
				lTempKind = (pEvent->m_lKind & 0x0F) | 0x80;
			}
			if (strcmp (g_szMIDIKindName[(lTempKind - 0x80) >> 4], "") != 0) {
				lOutput += sprintf (pBuf + lOutput, "%-17.17s", g_szMIDIKindName[(lTempKind - 0x80) >> 4]);
			}
			else {
				lOutput += sprintf (pBuf + lOutput, "0x%02lX             ", lTempKind);
			}
		}
		else if (MIDIEvent_IsSysExEvent (pEvent)) {
			if (strcmp (g_szSysExKindName[pEvent->m_lKind - 0xF0], "") != 0) {
				lOutput += sprintf (pBuf + lOutput, "%-17.17s", g_szSysExKindName[pEvent->m_lKind - 0xF0]);
			}
			else {
				lOutput += sprintf (pBuf + lOutput, "0x%02lX             ", pEvent->m_lKind);
			}
		}
		else {
				lOutput += sprintf (pBuf + lOutput, "0x%02lX             ", pEvent->m_lKind);
		}
	}
	/* �f�[�^���̒����̏o�� */
	if (lFlags & MIDIEVENT_DUMPLEN) {
		if (lColumn != 0) {
			lOutput += sprintf (pBuf + lOutput, " ");
		}
		lColumn++;
		lOutput += sprintf (pBuf + lOutput, "%4ld", pEvent->m_lLen);
	}
	/* �f�[�^���̏o�� */
	if (lFlags & MIDIEVENT_DUMPDATA) {
		long i;
		long n;
		unsigned char* p1;
		unsigned char* p2;
		if (lColumn != 0) {
			lOutput += sprintf (pBuf + lOutput, " ");
		}
		lColumn++;
		if (MIDIEVENT_TEXTEVENT <= pEvent->m_lKind && 
			pEvent->m_lKind < MIDIEVENT_CHANNELPREFIX) {
			n = CLIP (0, pEvent->m_lLen, 2047);
			n = CLIP (0, n, lLen - 64);
			lOutput += sprintf (pBuf + lOutput, "{\"");
			if (pEvent->m_pData && n > 0) { /* 20091024�������ǉ� */
				memcpy (pBuf + lOutput, (char*)pEvent->m_pData, n);
			}
			lOutput += n;
			lOutput += sprintf (pBuf + lOutput, "\"}");
		}
		else if ((MIDIEVENT_CHANNELPREFIX <= pEvent->m_lKind && 
			pEvent->m_lKind < MIDIEVENT_SEQUENCERSPECIFIC) ||
			pEvent->m_lKind >= MIDIEVENT_SYSEXSTART) {
			switch (pEvent->m_lKind) {
			case MIDIEVENT_CHANNELPREFIX:
			case MIDIEVENT_PORTPREFIX:
				lOutput += sprintf (pBuf + lOutput, "{%d}", *(pEvent->m_pData));
				break;
			case MIDIEVENT_ENDOFTRACK:
				lOutput += sprintf (pBuf + lOutput, "{}");
				break;
			case MIDIEVENT_TEMPO:
				lOutput += sprintf (pBuf + lOutput, "{%ld[microsec/beat](%ld[BPM])}", 
					MIDIEvent_GetTempo (pEvent), 60000000 / MIDIEvent_GetTempo (pEvent));
				break;
			case MIDIEVENT_SMPTEOFFSET:
				lOutput += sprintf (pBuf + lOutput, "{%d[h] %d[m] %d[s] %d[f] %d[ff]}", 
					*(pEvent->m_pData + 0), *(pEvent->m_pData + 1),
					*(pEvent->m_pData + 2), *(pEvent->m_pData + 3), *(pEvent->m_pData + 4));
				break;
			case MIDIEVENT_TIMESIGNATURE:
				lOutput += sprintf (pBuf + lOutput, "{%d / %d %d[clock/beat] %d[32divnote/beat]}", 
					*(pEvent->m_pData + 0), 1 << *(pEvent->m_pData + 1),
					*(pEvent->m_pData + 2), *(pEvent->m_pData + 3));
				break;
			case MIDIEVENT_KEYSIGNATURE:
				lOutput += sprintf (pBuf + lOutput, "{%d%s %s}",
					abs (*((char*)(pEvent->m_pData))), 
					*(pEvent->m_pData) == 0 ? "" : (*(pEvent->m_pData) < 128 ? "#" : "b"),
					*(pEvent->m_pData + 1) ? "minor" : "major");
				break;
			default:
				p1 = szText;
				p2 = pEvent->m_pData;
				n = CLIP (0, pEvent->m_lLen, 399);
				n = CLIP (0, pEvent->m_lLen, lLen - 64);
				for (i = 0; i < n; i++) {
					sprintf ((char*)p1, " 0x%02X", *p2++);
					p1 += 5;
				}
				lOutput += sprintf (pBuf + lOutput, "{%s}", szText + 1);
				break;
			}
		}
		else if (MIDIEvent_IsMIDIEvent (pEvent)) {
			switch (pEvent->m_lKind & 0xF0) {
			case MIDIEVENT_NOTEOFF:
			case MIDIEVENT_NOTEON:
				lOutput += sprintf (pBuf + lOutput, "{0x%02X  %-2s%d %4d %9ld}",
				*(pEvent->m_pData), 
				g_szKeyName[*(pEvent->m_pData + 1) % 12],
				*(pEvent->m_pData + 1) / 12, *(pEvent->m_pData + 2),
				MIDIEvent_IsNote (pEvent) ? MIDIEvent_GetDuration (pEvent) : 0);
				break;
			case MIDIEVENT_KEYAFTERTOUCH:
				lOutput += sprintf (pBuf + lOutput, "{0x%02X  %-2s%d %4d}",
				*(pEvent->m_pData), g_szKeyName[*(pEvent->m_pData + 1) % 12],
				*(pEvent->m_pData + 1) / 12, *(pEvent->m_pData + 2));
				break;
			case MIDIEVENT_CONTROLCHANGE:
				lOutput += sprintf (pBuf + lOutput, "{0x%02X %4d %4d}",
				*(pEvent->m_pData), *(pEvent->m_pData + 1), *(pEvent->m_pData + 2));
				break;
			case MIDIEVENT_PROGRAMCHANGE:
			case MIDIEVENT_CHANNELAFTERTOUCH:
				lOutput += sprintf (pBuf + lOutput, "{0x%02X %4d}",
				*(pEvent->m_pData), *(pEvent->m_pData + 1));
				break;
			case MIDIEVENT_PITCHBEND:
				lOutput += sprintf (pBuf + lOutput, "{0x%02X %9d}",
				*(pEvent->m_pData), *(pEvent->m_pData + 1) + *(pEvent->m_pData + 2) * 128);
				break;
			}
		}
		else if (MIDIEvent_IsSysExEvent (pEvent)) {
			p1 = szText;
			p2 = pEvent->m_pData;
			n = CLIP (0, pEvent->m_lLen, 399);
			n = CLIP (0, pEvent->m_lLen, lLen - 64);
			for (i = 0; i < n; i++) {
				sprintf ((char*)p1, " 0x%02X", *p2++);
				p1 += 5;
			}
			lOutput += sprintf (pBuf + lOutput, "{%s}", szText + 1);

		}
	}
	return pBuf;
}



/* �C�x���g�𕶎���\�����琶�� */
/* ���̊֐��͂܂��R�[�f�B���O����Ă��܂���B */
MIDIEvent* __stdcall MIDIEvent_FromStringEx (const char* pBuf, long lLen, long lFlags) {

	return NULL;
}

/* �C�x���g�𕶎���\�����琶�� */
/* ���̊֐��͂܂��R�[�f�B���O����Ă��܂���B */
MIDIEvent* __stdcall MIDIEvent_FromString (const char* pBuf, long lLen) {

	return NULL;
}


/******************************************************************************/
/*                                                                            */
/*�@MIDITrack�N���X�֐�                                                       */
/*                                                                            */
/******************************************************************************/

/* �g���b�N���̃C�x���g�̑������擾 */
long __stdcall MIDITrack_GetNumEvent (MIDITrack* pTrack) {
	return pTrack->m_lNumEvent;
}

/* �g���b�N�̍ŏ��̃C�x���g�擾(�Ȃ����NULL) */
MIDIEvent* __stdcall MIDITrack_GetFirstEvent (MIDITrack* pTrack) {
	assert (pTrack);
	return pTrack->m_pFirstEvent;
}

/* �g���b�N�̍Ō�̃C�x���g�擾(�Ȃ����NULL) */
/* �ʏ�A�Ō�̃C�x���g��EOT(�G���h�I�u�g���b�N)�ł��� */
MIDIEvent* __stdcall MIDITrack_GetLastEvent (MIDITrack* pTrack) {
	assert (pTrack);
	return pTrack->m_pLastEvent;
}

/* �g���b�N���̎w���ނ̍ŏ��̃C�x���g�擾(�Ȃ����NULL) */
MIDIEvent* __stdcall MIDITrack_GetFirstKindEvent (MIDITrack* pTrack, long lKind) {
	MIDIEvent* pEvent = NULL;
	assert (pTrack);
	forEachEvent (pTrack, pEvent) {
		if (pEvent->m_lKind == lKind) {
			return pEvent;
		}
	}
	return NULL;
}

/* �g���b�N���̎w���ނ̍Ō�̃C�x���g�擾(�Ȃ����NULL) */
MIDIEvent* __stdcall MIDITrack_GetLastKindEvent (MIDITrack* pTrack, long lKind) {
	MIDIEvent* pEvent = NULL;
	assert (pTrack);
	forEachEventInverse (pTrack, pEvent) {
		if (pEvent->m_lKind == lKind) {
			return pEvent;
		}
	}
	return NULL;
}

/* ����MIDI�g���b�N�ւ̃|�C���^�擾(�Ȃ����NULL)(20080715�ǉ�) */
MIDITrack* __stdcall MIDITrack_GetNextTrack (MIDITrack* pTrack) {
	assert (pTrack);
	return pTrack->m_pNextTrack;
}

/* �O��MIDI�g���b�N�ւ̃|�C���^�擾(�Ȃ����NULL)(20080715�ǉ�) */
MIDITrack* __stdcall MIDITrack_GetPrevTrack (MIDITrack* pTrack) {
	assert (pTrack);
	return pTrack->m_pPrevTrack;
}


/* �eMIDI�f�[�^�ւ̃|�C���^���擾(�Ȃ����NULL) */
MIDIData* __stdcall MIDITrack_GetParent (MIDITrack* pMIDITrack) {
	return (MIDIData*)(pMIDITrack->m_pParent);
}


/* �g���b�N���̃C�x���g�����J�E���g���A�e�C�x���g�̃C���f�b�N�X�Ƒ��C�x���g�����X�V���A�C�x���g����Ԃ��B */
long __stdcall MIDITrack_CountEvent (MIDITrack* pTrack) {
	long i = 0;
	MIDIEvent* pEvent;
	assert (pTrack);
	forEachEvent (pTrack, pEvent) {
		pEvent->m_lTempIndex = i;
		i++;
	}
	return pTrack->m_lNumEvent = i;
}

/* �g���b�N�̊J�n����(�ŏ��̃C�x���g�̎���)[Tick]���擾(20081101�ǉ�) */
long __stdcall MIDITrack_GetBeginTime (MIDITrack* pMIDITrack) {
	MIDIEvent* pFirstEvent = pMIDITrack->m_pFirstEvent;
	if (pFirstEvent) {
		return pFirstEvent->m_lTime;
	}
	return 0;
}

/* �g���b�N�̏I������(�Ō�̃C�x���g�̎���)[Tick]���擾(20081101�ǉ�) */
long __stdcall MIDITrack_GetEndTime (MIDITrack* pMIDITrack) {
	MIDIEvent* pLastEvent = pMIDITrack->m_pLastEvent;
	if (pLastEvent) {
		return pLastEvent->m_lTime;
	}
	return 0;
}

/* �g���b�N���擾(�g���b�N���C�x���g���Ȃ����NULL) */
char* __stdcall MIDITrack_GetName (MIDITrack* pTrack, char* pBuf, long lLen) {
	MIDIEvent* pEvent;
	memset (pBuf, '\0', lLen);
	forEachEvent (pTrack, pEvent) {
		if (pEvent->m_lKind == MIDIEVENT_TRACKNAME) {
			return MIDIEvent_GetText (pEvent, pBuf, lLen);
		}
	}
	return NULL;
}


/* ���͎擾(0=OFF, 1=On) */
long __stdcall MIDITrack_GetInputOn (MIDITrack* pTrack) {
	assert (pTrack);
	return pTrack->m_lInputOn;
}

/* ���̓|�[�g�擾(-1=n/a, 0�`15=�|�[�g�ԍ�) */
long __stdcall MIDITrack_GetInputPort (MIDITrack* pTrack) {
	assert (pTrack);
	return pTrack->m_lInputPort;
}

/* ���̓`�����l���擾(-1=n/a, 0�`15=�`�����l���ԍ�) */
long __stdcall MIDITrack_GetInputChannel (MIDITrack* pTrack) {
	assert (pTrack);
	return pTrack->m_lInputChannel;
}

/* �o�͎擾(0=OFF, 1=On) */
long __stdcall MIDITrack_GetOutputOn (MIDITrack* pTrack) {
	assert (pTrack);
	return pTrack->m_lOutputOn;
}

/* �o�̓|�[�g(-1=n/a, 0�`15=�|�[�g�ԍ�) */
long __stdcall MIDITrack_GetOutputPort (MIDITrack* pTrack) {
	assert (pTrack);
	return pTrack->m_lOutputPort;
}

/* �o�̓`�����l��(-1=n/a, 0�`15=�`�����l���ԍ�) */
long __stdcall MIDITrack_GetOutputChannel (MIDITrack* pTrack) {
	assert (pTrack);
	return pTrack->m_lOutputChannel;
}

/* �^�C��+�擾 */
long __stdcall MIDITrack_GetTimePlus (MIDITrack* pTrack) {
	assert (pTrack);
	return pTrack->m_lTimePlus;
}

/* �L�[+�擾 */
long __stdcall MIDITrack_GetKeyPlus (MIDITrack* pTrack) {
	assert (pTrack);
	return pTrack->m_lKeyPlus;
}

/* �x���V�e�B+�擾 */
long __stdcall MIDITrack_GetVelocityPlus (MIDITrack* pTrack) {
	assert (pTrack);
	return pTrack->m_lVelocityPlus;
}

/* �\�����[�h�擾(0=�ʏ�A1=�h����) */
long __stdcall MIDITrack_GetViewMode (MIDITrack* pTrack) {
	assert (pTrack);
	return pTrack->m_lViewMode;
}

/* �O�i�F�擾 */
long __stdcall MIDITrack_GetForeColor (MIDITrack* pTrack) {
	assert (pTrack);
	return pTrack->m_lForeColor;
}

/* �w�i�F�擾 */
long __stdcall MIDITrack_GetBackColor (MIDITrack* pTrack) {
	assert (pTrack);
	return pTrack->m_lBackColor;
}


/* �g���b�N���ݒ�(�g���b�N���C�x���g���Ȃ���Ύ����ǉ�) */
long __stdcall MIDITrack_SetName (MIDITrack* pTrack, const char* pszText) {
	MIDIEvent* pEvent;
	long lLen;
	lLen = strlen (pszText);
	forEachEvent (pTrack, pEvent) {
		if (pEvent->m_lKind == MIDIEVENT_TRACKNAME) {
			MIDIEvent_SetText (pEvent, pszText);
			return 1;
		}
	}
	return MIDITrack_InsertTrackName (pTrack, 0, pszText);
}


/* ���͐ݒ�(0=OFF, 1=On) */
long __stdcall MIDITrack_SetInputOn (MIDITrack* pTrack, long lInputOn) {
	assert (pTrack);
	pTrack->m_lInputOn = lInputOn;
	return 1;
}

/* ���̓|�[�g�ݒ�(-1=n/a, 0�`15=�|�[�g�ԍ�) */
long __stdcall MIDITrack_SetInputPort (MIDITrack* pTrack, long lInputPort) {
	assert (pTrack);
	pTrack->m_lInputPort = CLIP (0, lInputPort, 255);
	return 1;
}

/* ���̓`�����l���ݒ�(-1=n/a, 0�`15=�`�����l���ԍ�) */
long __stdcall MIDITrack_SetInputChannel (MIDITrack* pTrack, long lInputChannel) {
	assert (pTrack);
	pTrack->m_lInputChannel = CLIP (-1, lInputChannel, 15);
	return 1;
}

/* �o�͐ݒ�(0=OFF, 1=On) */
long __stdcall MIDITrack_SetOutputOn (MIDITrack* pTrack, long lOutputOn) {
	assert (pTrack);
	pTrack->m_lOutputOn = lOutputOn;
	return 1;
}

/* �o�̓|�[�g(-1=n/a, 0�`15=�|�[�g�ԍ�) */
long __stdcall MIDITrack_SetOutputPort (MIDITrack* pTrack, long lOutputPort) {
	assert (pTrack);
	pTrack->m_lOutputPort = CLIP (0, lOutputPort, 255);
	return 1;
}

/* �o�̓`�����l��(-1=n/a, 0�`15=�`�����l���ԍ�) */
long __stdcall MIDITrack_SetOutputChannel (MIDITrack* pTrack, long lOutputChannel) {
	assert (pTrack);
	pTrack->m_lOutputChannel = CLIP (-1, lOutputChannel, 15);
	return 1;
}

/* �^�C��+�ݒ� */
long __stdcall MIDITrack_SetTimePlus (MIDITrack* pTrack, long lTimePlus) {
	assert (pTrack);
	pTrack->m_lTimePlus = lTimePlus;
	return 1;
}

/* �L�[+�ݒ� */
long __stdcall MIDITrack_SetKeyPlus (MIDITrack* pTrack, long lKeyPlus) {
	assert (pTrack);
	pTrack->m_lKeyPlus = lKeyPlus;
	return 1;
}

/* �x���V�e�B+�ݒ� */
long __stdcall MIDITrack_SetVelocityPlus (MIDITrack* pTrack, long lVelocityPlus) {
	assert (pTrack);
	pTrack->m_lVelocityPlus = lVelocityPlus;
	return 1;
}

/* �\�����[�h�ݒ�(0=�ʏ�A1=�h����) */
long __stdcall MIDITrack_SetViewMode (MIDITrack* pTrack, long lViewMode) {
	assert (pTrack);
	pTrack->m_lViewMode = lViewMode;
	return 1;
}

/* �O�i�F�ݒ� */
long __stdcall MIDITrack_SetForeColor (MIDITrack* pTrack, long lForeColor) {
	assert (pTrack);
	pTrack->m_lForeColor = lForeColor;
	return 1;
}

/* �w�i�F�ݒ� */
long __stdcall MIDITrack_SetBackColor (MIDITrack* pTrack, long lBackColor) {
	assert (pTrack);
	pTrack->m_lBackColor = lBackColor;
	return 1;
}

/* XF�ł���Ƃ��AXF�̃��@�[�W�������擾(XF�łȂ����0) */
long __stdcall MIDITrack_GetXFVersion (MIDITrack* pMIDITrack) {
	MIDIEvent* pMIDIEvent = NULL;
	/* �V�[�P���T�ŗL�̃C�x���g�� */
	/* {43 7B 00 58 46 Mj Mn S1 S0} ��T���A*/
	/* Mj | (Mn << 8) | (S1 << 16) | (S0 << 24) ��Ԃ��B*/
	forEachEvent (pMIDITrack, pMIDIEvent) {
		if (MIDIEvent_IsSequencerSpecific (pMIDIEvent)) {
			if (MIDIEvent_GetLen (pMIDIEvent) == 9) {
				if (*(pMIDIEvent->m_pData + 0) == 0x43 &&
					*(pMIDIEvent->m_pData + 1) == 0x7B &&
					*(pMIDIEvent->m_pData + 2) == 0x00 &&
					*(pMIDIEvent->m_pData + 3) == 0x58 &&
					*(pMIDIEvent->m_pData + 4) == 0x46) {
					return 
						(*(pMIDIEvent->m_pData + 5) << 0) |
						(*(pMIDIEvent->m_pData + 6) << 8) |	
						(*(pMIDIEvent->m_pData + 7) << 16) |	
						(*(pMIDIEvent->m_pData + 8) << 24);
				}
			}
		}
	}
	/* XF�ł͂Ȃ� */
	return 0;
}




/* �g���b�N�̍폜(�g���b�N���Ɋ܂܂��C�x���g�I�u�W�F�N�g���폜����܂�) */
void __stdcall MIDITrack_Delete (MIDITrack* pTrack) {
	MIDIEvent* pEvent;
	MIDIEvent* pNextEvent;
	assert (pTrack);
	/* �g���b�N���̃C�x���g�폜 */
	pEvent = MIDITrack_GetFirstEvent (pTrack);
	while (pEvent) {
		pNextEvent = MIDIEvent_GetNextEvent (pEvent);
		MIDIEvent_DeleteSingle (pEvent);
		pEvent = pNextEvent;
	}
	/* �o�������X�g�|�C���^�̂Ȃ����� */
	if (pTrack->m_pNextTrack) {
		pTrack->m_pNextTrack->m_pPrevTrack = pTrack->m_pPrevTrack;
	}
	else if (pTrack->m_pParent) {
		((MIDIData*)(pTrack->m_pParent))->m_pLastTrack = pTrack->m_pPrevTrack;
	}

	if (pTrack->m_pPrevTrack) {
		pTrack->m_pPrevTrack->m_pNextTrack = pTrack->m_pNextTrack;
	}
	else if (pTrack->m_pParent) {
		((MIDIData*)(pTrack->m_pParent))->m_pFirstTrack = pTrack->m_pNextTrack;
	}

	if (pTrack->m_pParent) {
		((MIDIData*)(pTrack->m_pParent))->m_lNumTrack --;
		pTrack->m_pParent = NULL;
	}
	free (pTrack);
}

/* ��̃g���b�N�̐��� */
/* �G���h�I�u�g���b�N�C�x���g�͎蓮�ŕt�����Ȃ���΂Ȃ�Ȃ��B*/
MIDITrack* __stdcall MIDITrack_Create () {
	MIDITrack* pTrack = NULL;
	pTrack = calloc (1, sizeof (MIDITrack));
	if (pTrack == NULL) {
		return NULL;
	}
	pTrack->m_lNumEvent = 0;
	pTrack->m_pFirstEvent = NULL;
	pTrack->m_pLastEvent = NULL;
	pTrack->m_pNextTrack = NULL;
	pTrack->m_pPrevTrack = NULL;
	pTrack->m_pParent = NULL;
	pTrack->m_lInputOn = 1;
	pTrack->m_lInputPort = 0;
	pTrack->m_lInputChannel = 0;
	pTrack->m_lOutputOn = 1;
	pTrack->m_lOutputPort = 0;
	pTrack->m_lOutputChannel = 0;
	pTrack->m_lTimePlus = 0;
	pTrack->m_lKeyPlus = 0;
	pTrack->m_lVelocityPlus = 0;
	pTrack->m_lViewMode = 0;
	pTrack->m_lForeColor = 0x00000000;
	pTrack->m_lBackColor = 0x00FFFFFF;
	pTrack->m_lUser1 = 0;
	pTrack->m_lUser2 = 0;
	pTrack->m_lUser3 = 0;
	pTrack->m_lUserFlag = 0;
	return pTrack;
}

/* MIDI�g���b�N�̃N���[���𐶐� */
MIDITrack* __stdcall MIDITrack_CreateClone (MIDITrack* pTrack) {
	MIDITrack* pCloneTrack = NULL;
	MIDIEvent* pSrcEvent = NULL;
	MIDIEvent* pCloneEvent = NULL;
	long lCount = MIDITrack_CountEvent (pTrack);
	pCloneTrack = MIDITrack_Create ();
	if (pCloneTrack == NULL) {
		return NULL;
	}
	forEachEvent (pTrack, pSrcEvent) {
		if (pSrcEvent->m_pPrevCombinedEvent == NULL) {
			pCloneEvent = MIDIEvent_CreateClone (pSrcEvent);
			if (pCloneEvent == NULL) {
				MIDITrack_Delete (pCloneTrack);
				return NULL;
			}
			MIDITrack_InsertEvent (pCloneTrack, pCloneEvent);
		}
	}
	/* TODO �C�x���g�̏������������Ă���̂ŕ��בւ���(m_lTempIndex�̒l��p����) */
	pCloneTrack->m_lInputOn = pTrack->m_lInputOn;
	pCloneTrack->m_lInputPort = pTrack->m_lInputPort;
	pCloneTrack->m_lInputChannel = pTrack->m_lInputChannel;
	pCloneTrack->m_lOutputOn = pTrack->m_lOutputOn;
	pCloneTrack->m_lOutputPort = pTrack->m_lOutputPort;
	pCloneTrack->m_lOutputChannel = pTrack->m_lOutputChannel;
	pCloneTrack->m_lTimePlus = pTrack->m_lTimePlus;
	pCloneTrack->m_lKeyPlus = pTrack->m_lKeyPlus;
	pCloneTrack->m_lVelocityPlus = pTrack->m_lVelocityPlus;
	pCloneTrack->m_lViewMode = pTrack->m_lViewMode;
	pCloneTrack->m_lForeColor = pTrack->m_lForeColor;
	pCloneTrack->m_lBackColor = pTrack->m_lBackColor;
	pCloneTrack->m_lReserved1 = pTrack->m_lReserved1;
	pCloneTrack->m_lReserved2 = pTrack->m_lReserved2;
	pCloneTrack->m_lReserved3 = pTrack->m_lReserved3;
	pCloneTrack->m_lReserved4 = pTrack->m_lReserved4;
	pCloneTrack->m_lUser1 = pTrack->m_lUser1;
	pCloneTrack->m_lUser2 = pTrack->m_lUser2;
	pCloneTrack->m_lUser3 = pTrack->m_lUser3;
	pCloneTrack->m_lUserFlag = pTrack->m_lUserFlag;
	return pCloneTrack;
}

/* �g���b�N�Ƀm�[�g�I�t�C�x���g�𐳂����}�� */
/* �������Ă���m�[�g�I���C�x���g�͊��ɑ}���ς݂Ƃ���B */
/* �������Ƀm�[�g�I�t�C�x���g������ꍇ�͂����̒��O�ɑ}������ */
/* (���̊֐��͓����B������Ă��܂��B) */
long MIDITrack_InsertNoteOffEventBefore (MIDITrack* pTrack, MIDIEvent* pNoteOffEvent) {
	MIDIEvent* pOldEvent = NULL;
	MIDIEvent* pTempEvent = NULL;
	MIDIEvent* pNoteOnEvent = pNoteOffEvent->m_pPrevCombinedEvent;
	assert (pTrack);
	assert (pNoteOnEvent);
	assert (!MIDIEvent_IsFloating (pNoteOnEvent));
	pOldEvent = pNoteOnEvent;
	pTempEvent = pNoteOnEvent->m_pNextEvent;
	while (pTempEvent) {
		if (pTempEvent->m_lKind == MIDIEVENT_ENDOFTRACK && 
			pTempEvent->m_pNextEvent == NULL) {
			pTempEvent->m_lTime = pNoteOffEvent->m_lTime;
			break;
		}
		else if (pTempEvent->m_lTime >= pNoteOffEvent->m_lTime) {
			break;
		}
		pOldEvent = pTempEvent;
		pTempEvent = pTempEvent->m_pNextEvent;
	}
	return MIDIEvent_SetNextEvent (pOldEvent, pNoteOffEvent);
}


/* �g���b�N�Ƀm�[�g�I�t�C�x���g�𐳂����}�� */
/* �������Ă���m�[�g�I���C�x���g�͊��ɑ}���ς݂Ƃ���B */
/* �������Ƀm�[�g�I�t�C�x���g������ꍇ�͂����̒���ɑ}������ */
/* (���̊֐��͓����B������Ă��܂��B) */
long MIDITrack_InsertNoteOffEventAfter (MIDITrack* pTrack, MIDIEvent* pNoteOffEvent) {
	MIDIEvent* pOldEvent = NULL;
	MIDIEvent* pTempEvent = NULL;
	MIDIEvent* pNoteOnEvent = pNoteOffEvent->m_pPrevCombinedEvent;
	assert (pTrack);
	assert (pNoteOnEvent);
	assert (!MIDIEvent_IsFloating (pNoteOnEvent));
	pOldEvent = pNoteOnEvent;
	pTempEvent = pNoteOnEvent->m_pNextEvent;
	while (pTempEvent) {
		if (pTempEvent->m_lKind == MIDIEVENT_ENDOFTRACK && 
			pTempEvent->m_pNextEvent == NULL) {
			pTempEvent->m_lTime = pNoteOffEvent->m_lTime;
			break;
		}
		else if (pTempEvent->m_lTime > pNoteOffEvent->m_lTime ||
			(pTempEvent->m_lTime == pNoteOffEvent->m_lTime &&
			!MIDIEvent_IsNoteOff (pTempEvent))) {
			break;
		}
		pOldEvent = pTempEvent;
		pTempEvent = pTempEvent->m_pNextEvent;
	}
	return MIDIEvent_SetNextEvent (pOldEvent, pNoteOffEvent);
}


/* �g���b�N�ɒP��̃C�x���g��}�� */
/* pEvent��pTarget�̒��O�ɓ����B�������s���ȏꍇ�A������������B*/
/* pTarget==NULL�̏ꍇ�A�g���b�N�̍Ō�ɓ����B */
/* (���̊֐��͓����B������Ă��܂��B) */
long __stdcall MIDITrack_InsertSingleEventBefore 
(MIDITrack* pTrack, MIDIEvent* pEvent, MIDIEvent* pTarget) {
	assert (pTrack);
	assert (pEvent);
	/* �C�x���g�����ɑ��̃g���b�N�ɑ����Ă���ꍇ�A�p������ */
	if (pEvent->m_pParent || pEvent->m_pPrevEvent || pEvent->m_pNextEvent) {
		return 0;
	}
	/* EOT���d�ɓ����̂�h�~ */
	if (pTrack->m_pLastEvent) {
		if (pTrack->m_pLastEvent->m_lKind == MIDIEVENT_ENDOFTRACK &&
			pEvent->m_lKind == MIDIEVENT_ENDOFTRACK) {
			return 0;
		}
	}
	/* SMF�t�H�[�}�b�g1�̏ꍇ */
	if (pTrack->m_pParent) {
		if (((MIDIData*)(pTrack->m_pParent))->m_lFormat == 1) {
			/* �R���_�N�^�[�g���b�N��MIDIEvent������̂�h�~ */
			if (((MIDIData*)(pTrack->m_pParent))->m_pFirstTrack == pTrack) {
				if (MIDIEvent_IsMIDIEvent (pEvent)) {
					return 0;
				}
			}
			/* ��R���_�N�^�[�g���b�N�Ƀe���|�E���q�Ȃǂ�����̂�h�~ */
			else {
				if (pEvent->m_lKind == MIDIEVENT_TEMPO ||
					pEvent->m_lKind == MIDIEVENT_SMPTEOFFSET ||
					pEvent->m_lKind == MIDIEVENT_TIMESIGNATURE ||
					pEvent->m_lKind == MIDIEVENT_KEYSIGNATURE) {
					return 0;
				}
			}
		}
	}
	/* pTarget�̒��O�ɑ}������ꍇ */
	if (pTarget) {
		/* �^�[�Q�b�g�������g���b�N���قȂ�ꍇ�p�� */
		if (pTarget->m_pParent != pTrack) {
			assert (0);
			return 0;
		}
		MIDIEvent_SetPrevEvent (pTarget, pEvent);
	}
	/* �g���b�N�̍Ō�ɑ}������ꍇ(pTarget==NULL) */
	else if (pTrack->m_pLastEvent) {
		/* EOT�̌�ɑ}�����悤�Ƃ����ꍇ�AEOT�����Ɉړ���EOT�̒��O�ɑ}�� */
		if (pTrack->m_pLastEvent->m_lKind == MIDIEVENT_ENDOFTRACK) {
			/* EOT�𐳂����ړ����邽�߁A��Ɏ����̐������� */
			if (pTrack->m_pLastEvent->m_lTime < pEvent->m_lTime) {
				pTrack->m_pLastEvent->m_lTime = pEvent->m_lTime;
			}
			MIDIEvent_SetPrevEvent (pTrack->m_pLastEvent, pEvent);
		}
		/* EOT�ȊO�̌�ɑ}�����悤�Ƃ����ꍇ�A���ʂɑ}�� */
		else {
			MIDIEvent_SetNextEvent (pTrack->m_pLastEvent, pEvent);
		}
	}
	/* ��g���b�N�ɑ}������ꍇ */
	else {
		pEvent->m_pParent = pTrack;
		pEvent->m_pNextEvent = NULL;
		pEvent->m_pPrevEvent = NULL;
		pEvent->m_pNextSameKindEvent = NULL;
		pEvent->m_pPrevSameKindEvent = NULL;
		pTrack->m_pFirstEvent = pEvent;
		pTrack->m_pLastEvent = pEvent;
		pTrack->m_lNumEvent ++;
	}
	return 1;
}

/* �g���b�N�ɃC�x���g��}��(�����C�x���g�ɂ��Ή�) */
/* pEvent��pTarget�̒��O�ɓ����B�������s���ȏꍇ�A������������B*/
/* pTarget==NULL�̏ꍇ�A�g���b�N�̍Ō�ɓ����B */
long __stdcall MIDITrack_InsertEventBefore (MIDITrack* pTrack, MIDIEvent* pEvent, MIDIEvent* pTarget) {
	long lRet = 0;
	assert (pTrack);
	assert (pEvent);
	/* �񕂗V�C�x���g�͑}���ł��Ȃ��B */
	if (!MIDIEvent_IsFloating (pEvent)) {
		return 0;
	}
	pEvent = MIDIEvent_GetFirstCombinedEvent (pEvent);
	/* �m�[�g�C�x���g�ȊO�̌����C�x���g�̊Ԃɂ͑}���ł��Ȃ� */
	if (pTarget) {
		if (!MIDIEvent_IsNote (pTarget)) {
			pTarget = MIDIEvent_GetFirstCombinedEvent (pTarget);
		}
	}
	/* �P�Ƃ̃C�x���g�̏ꍇ */
	if (MIDIEvent_IsCombined (pEvent) == 0) {
		lRet = MIDITrack_InsertSingleEventBefore (pTrack, pEvent, pTarget);
		return lRet;
	}
	/* �m�[�g�C�x���g�̏ꍇ */
	else if (MIDIEvent_IsNote (pEvent)) {
		lRet = MIDITrack_InsertSingleEventBefore (pTrack, pEvent, pTarget);
		if (lRet == 0) {
			return 0;
		}
		lRet = MIDITrack_InsertNoteOffEventBefore (pTrack, pEvent->m_pNextCombinedEvent);
		if (lRet == 0) {
			MIDITrack_RemoveSingleEvent (pTrack, pEvent);
			return 0;
		}
		return 2;
	}
	/* RPN�`�F���W����NPRN�`�F���W���̓p�b�`�`�F���W�̏ꍇ */
	else if (MIDIEvent_IsRPNChange (pEvent) || MIDIEvent_IsNRPNChange (pEvent) ||
	MIDIEvent_IsPatchChange (pEvent)) {
		lRet = MIDITrack_InsertSingleEventBefore (pTrack, pEvent, pTarget);
		if (lRet == 0) {
			return 0;
		}
		lRet = MIDITrack_InsertSingleEventBefore (pTrack, pEvent->m_pNextCombinedEvent, pTarget);
		if (lRet == 0) {
			MIDITrack_RemoveSingleEvent (pTrack, pEvent);
			return 0;
		}
		lRet = MIDITrack_InsertSingleEventBefore (pTrack, pEvent->m_pNextCombinedEvent->m_pNextCombinedEvent, pTarget);
		if (lRet == 0) {
			MIDITrack_RemoveSingleEvent (pTrack, pEvent);
			MIDITrack_RemoveSingleEvent (pTrack, pEvent->m_pNextCombinedEvent);
			return 0;
		}
		return 3;
	}
	/* ����`�̌����C�x���g */
	assert (0);
	return 0;
}



/* �g���b�N�ɃC�x���g��}��(�C�x���g�͂��炩���ߐ������Ă���) */
/* pEvent��pTarget�̒���ɓ����B�������s���ȏꍇ�A������������B*/
/* pTarget==NULL�̏ꍇ�A�g���b�N�̍ŏ��ɓ����B */
long __stdcall MIDITrack_InsertSingleEventAfter
	(MIDITrack* pTrack, MIDIEvent* pEvent, MIDIEvent* pTarget) {
	assert (pTrack);
	assert (pEvent);
	/* �C�x���g�����ɑ��̃g���b�N�ɑ����Ă���ꍇ�A�p������ */
	if (pEvent->m_pParent || pEvent->m_pPrevEvent || pEvent->m_pNextEvent) {
		return 0;
	}
	/* EOT���d�ɓ����̂�h�~ */
	if (pTrack->m_pLastEvent) {
		if (pTrack->m_pLastEvent->m_lKind == MIDIEVENT_ENDOFTRACK &&
			pEvent->m_lKind == MIDIEVENT_ENDOFTRACK) {
			return 0;
		}
	}
	/* SMF�t�H�[�}�b�g1�̏ꍇ */
	if (pTrack->m_pParent) {
		if (((MIDIData*)(pTrack->m_pParent))->m_lFormat == 1) {
			/* �R���_�N�^�[�g���b�N��MIDIEvent������̂�h�~ */
			if (((MIDIData*)(pTrack->m_pParent))->m_pFirstTrack == pTrack) {
				if (MIDIEvent_IsMIDIEvent (pEvent)) {
					return 0;
				}
			}
			/* ��R���_�N�^�[�g���b�N�Ƀe���|�E���q�Ȃǂ�����̂�h�~ */
			else {
				if (pEvent->m_lKind == MIDIEVENT_TEMPO ||
					pEvent->m_lKind == MIDIEVENT_SMPTEOFFSET ||
					pEvent->m_lKind == MIDIEVENT_TIMESIGNATURE ||
					pEvent->m_lKind == MIDIEVENT_KEYSIGNATURE) {
					return 0;
				}
			}
		}
	}
	
	/* pTarget�̒���ɑ}������ꍇ */
	if (pTarget) {
		/* �^�[�Q�b�g�������g���b�N���قȂ�ꍇ�p�� */
		if (pTarget->m_pParent != pTrack) {
			assert (0);
			return 0;
		}
		/* EOT�̒���ɑ}�����悤�Ƃ����ꍇ�AEOT���ړ���EOT�̒��O�ɑ}�� */
		if (pTarget->m_lKind == MIDIEVENT_ENDOFTRACK &&
			pTarget->m_pNextEvent == NULL) {
			/* EOT�𐳂����ړ����邽�߁A��Ɏ����̐������� */
			if (pTarget->m_lTime < pEvent->m_lTime) {
				pTarget->m_lTime = pEvent->m_lTime;
			}
			MIDIEvent_SetPrevEvent (pTarget, pEvent);
		}
		/* EOT�ȊO�̒���ɑ}�����悤�Ƃ����ꍇ�A�����̐�����������Ή\(pTarget==NULL) */
		else {
			if (pTrack->m_pLastEvent->m_lKind == MIDIEVENT_ENDOFTRACK) {
				if (pTrack->m_pLastEvent->m_lTime < pEvent->m_lTime) {
					pTrack->m_pLastEvent->m_lTime = pEvent->m_lTime;
				}
			}
			MIDIEvent_SetNextEvent (pTarget, pEvent);
		}
	}
	/* �g���b�N�̍ŏ��ɑ}������ꍇ(pTarget==NULL) */
	else if (pTrack->m_pFirstEvent) {
		/* EOT�̒��O�ƂȂ�ꍇ�́AEOT�̎����𒲐����� */
		if (pTrack->m_pFirstEvent->m_lKind == MIDIEVENT_ENDOFTRACK &&
			pTrack->m_pFirstEvent->m_pNextEvent == NULL) {
			if (pTrack->m_pFirstEvent->m_lTime < pEvent->m_lTime) {
				pTrack->m_pFirstEvent->m_lTime = pEvent->m_lTime;
			}
		}
		MIDIEvent_SetPrevEvent (pTrack->m_pFirstEvent, pEvent);
	}
	/* ��g���b�N�ɑ}������ꍇ */
	else {
		pEvent->m_pParent = pTrack;
		pEvent->m_pNextEvent = NULL;
		pEvent->m_pPrevEvent = NULL;
		pEvent->m_pNextSameKindEvent = NULL;
		pEvent->m_pPrevSameKindEvent = NULL;
		pTrack->m_pFirstEvent = pEvent;
		pTrack->m_pLastEvent = pEvent;
		pTrack->m_lNumEvent ++;
	}
	return 1;
}

/* �g���b�N�ɃC�x���g��}��(�����C�x���g�ɂ��Ή�) */
/* pEvent��pTarget�̒��O�ɓ����B�������s���ȏꍇ�A������������B*/
/* pTarget==NULL�̏ꍇ�A�g���b�N�̍Ō�ɓ����B */
/* (���̊֐��͓����B������Ă��܂��B) */
long __stdcall MIDITrack_InsertEventAfter (MIDITrack* pTrack, MIDIEvent* pEvent, MIDIEvent* pTarget) {
	long lRet = 0;
	assert (pTrack);
	assert (pEvent);
	/* �񕂗V�C�x���g�͑}���ł��Ȃ��B */
	if (!MIDIEvent_IsFloating (pEvent)) {
		return 0;
	}
	pEvent = MIDIEvent_GetLastCombinedEvent (pEvent);
	/* �m�[�g�C�x���g�ȊO�̌����C�x���g�̊Ԃɂ͑}���ł��Ȃ� */
	if (pTarget) {
		if (!MIDIEvent_IsNote (pTarget)) {
			pTarget = MIDIEvent_GetLastCombinedEvent (pTarget);
		}
	}
	/* �P�Ƃ̃C�x���g�̏ꍇ */
	if (!MIDIEvent_IsCombined (pEvent)) {
		lRet = MIDITrack_InsertSingleEventAfter (pTrack, pEvent, pTarget);
		return lRet;
	}
	/* �m�[�g�C�x���g�̏ꍇ */
	else if (MIDIEvent_IsNote (pEvent)) {
		lRet = MIDITrack_InsertSingleEventAfter (pTrack, pEvent->m_pPrevCombinedEvent, pTarget);
		if (lRet == 0) {
			return 0;
		}
		lRet = MIDITrack_InsertNoteOffEventAfter (pTrack, pEvent);
		if (lRet == 0) {
			MIDITrack_RemoveSingleEvent (pTrack, pEvent->m_pPrevCombinedEvent);
			return 0;
		}
		return 2;
	}
	/* RPN�`�F���W����NPRN�`�F���W���̓p�b�`�`�F���W�̏ꍇ */
	else if (MIDIEvent_IsRPNChange (pEvent) || MIDIEvent_IsNRPNChange (pEvent) ||
	MIDIEvent_IsPatchChange (pEvent)) {
		lRet = MIDITrack_InsertSingleEventAfter (pTrack, pEvent, pTarget);
		if (lRet == 0) {
			return 0;
		}
		lRet = MIDITrack_InsertSingleEventAfter (pTrack, pEvent->m_pPrevCombinedEvent, pTarget);
		if (lRet == 0) {
			MIDITrack_RemoveSingleEvent (pTrack, pEvent);
			return 0;
		}
		lRet = MIDITrack_InsertSingleEventAfter (pTrack, pEvent->m_pPrevCombinedEvent->m_pPrevCombinedEvent, pTarget);
		if (lRet == 0) {
			MIDITrack_RemoveSingleEvent (pTrack, pEvent);
			MIDITrack_RemoveSingleEvent (pTrack, pEvent->m_pPrevCombinedEvent);
			return 0;
		}
		return 3;
	}
	/* ����`�̌����C�x���g */
	assert (0);
	return 0;
}

/* �g���b�N�ɃC�x���g��}��(�C�x���g�͂��炩���ߐ������Ă���) */
/* �}���ʒu�͎����ɂ�茈�肷��B*/
/* �������̃C�x���g������ꍇ�́A�����̍Ō�ɑ}������� */
long __stdcall MIDITrack_InsertEvent (MIDITrack* pTrack, MIDIEvent* pEvent) {
	long i = 0;
	long lRet = 0;
	MIDIEvent* pInsertEvent = pEvent;
	MIDIData* pMIDIData = NULL;
	assert (pTrack);
	assert (pEvent);
	/* pEvent�����V��Ԃł��邱�Ƃ��m�F */
	if (pEvent->m_pParent != NULL || pEvent->m_pPrevEvent != NULL || pEvent->m_pNextEvent != NULL) {
		return 0;
	}
	/* �G���h�I�u�g���b�N�̏d���}���̖h�~ */
	if (pTrack->m_pLastEvent) {
		if (((MIDIEvent*)(pTrack->m_pLastEvent))->m_lKind == MIDIEVENT_ENDOFTRACK && 
			pEvent->m_lKind == MIDIEVENT_ENDOFTRACK) {
			return 0;
		}
	}
	/* �t�H�[�}�b�g1�̂Ƃ��̏ꍇ�̃C�x���g�̎�ސ������`�F�b�N */
	pMIDIData = MIDITrack_GetParent (pTrack);
	if (pMIDIData) {
		if (pMIDIData->m_lFormat == MIDIDATA_FORMAT1) {
			/* �ŏ��̃g���b�N��MIDI�`�����l���C�x���g�̑}���h�~ */
			if (pTrack == pMIDIData->m_pFirstTrack) {
				if (0x80 <= pEvent->m_lKind && pEvent->m_lKind <= 0xEF) {
					return 0;	
				}
			}
			/* 2�Ԗڈȍ~�̃g���b�N�Ƀe���|�ESMPTE�I�t�Z�b�g�E���q�L���E�����L���̑}���h�~ */
			else {
				if (0x50 <= pEvent->m_lKind && pEvent->m_lKind <= 0x5F) {
					return 0;
				}
			}
		}
	}

	/* �e�C�x���g�̏��� */
	pInsertEvent = MIDIEvent_GetFirstCombinedEvent (pEvent);
	while (pInsertEvent) {
		MIDIEvent* pTempEvent = pTrack->m_pLastEvent;
		long lInsertTime = MIDIEvent_GetTime (pInsertEvent);
		/* �g���b�N�̌������}���ʒu��T�� */
		while (1) {
			/* �g���b�N�Ƀf�[�^���Ȃ��A���̓g���b�N�̐擪����Ă悢 */
			if (pTempEvent == NULL) {
				lRet += MIDITrack_InsertSingleEventAfter (pTrack, pInsertEvent, NULL);
				break;
			}
			/* pTempEvent�̒���ɓ���Ă悢 */
			else {
				/* �}��������̂��m�[�g�I�t�C�x���g�̏ꍇ(�x���V�e�B0�̃m�[�g�I�����܂�) */
				if (MIDIEvent_IsNoteOff (pInsertEvent)) {
					/* �Ή�����m�[�g�I���C�x���g���O�ɂ͐�΂ɗ���Ȃ� (20090111�ǉ�) */
					if (pTempEvent == pInsertEvent->m_pPrevCombinedEvent) {
						lRet += MIDITrack_InsertSingleEventAfter (pTrack, pInsertEvent, pTempEvent);
						break;
					}
					/* �������̃C�x���g������ꍇ�͓������̑��̃m�[�g�I�t�̒���ɑ}�� */
					else if (pTempEvent->m_lTime == lInsertTime && MIDIEvent_IsNoteOff (pTempEvent)) {
						lRet += MIDITrack_InsertSingleEventAfter (pTrack, pInsertEvent, pTempEvent);
						break;
					}
					else if (pTempEvent->m_lTime < lInsertTime) {
						lRet += MIDITrack_InsertSingleEventAfter (pTrack, pInsertEvent, pTempEvent);
						break;
					}
				}
				/* ���̑��̃C�x���g�̏ꍇ */
				else {
					if (pTempEvent->m_lTime <= lInsertTime) {
						lRet += MIDITrack_InsertSingleEventAfter (pTrack, pInsertEvent, pTempEvent);
						break;
					}
				}
			}
			pTempEvent = pTempEvent->m_pPrevEvent;
		}
		pInsertEvent = pInsertEvent->m_pNextCombinedEvent;
		i++;
	}
	return lRet;
}

/* �g���b�N�ɃV�[�P���X�ԍ��C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertSequenceNumber (MIDITrack* pTrack, long lTime, long lNum) { 
	MIDIEvent* pEvent = MIDIEvent_CreateSequenceNumber (lTime, lNum);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}


/* �g���b�N�Ƀe�L�X�g�C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertTextEvent (MIDITrack* pTrack, long lTime, const char* pszText) { 
	MIDIEvent* pEvent = MIDIEvent_CreateTextEvent (lTime, pszText);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�ɒ��쌠�C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertCopyrightNotice (MIDITrack* pTrack, long lTime, const char* pszText) { 
	MIDIEvent* pEvent = MIDIEvent_CreateCopyrightNotice (lTime, pszText);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�Ƀg���b�N���C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertTrackName (MIDITrack* pTrack, long lTime, const char* pszText) { 
	MIDIEvent* pEvent = MIDIEvent_CreateTrackName (lTime, pszText);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�ɃC���X�g�D�������g�C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertInstrumentName (MIDITrack* pTrack, long lTime, const char* pszText) { 
	MIDIEvent* pEvent = MIDIEvent_CreateInstrumentName (lTime, pszText);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�ɉ̎��C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertLyric (MIDITrack* pTrack, long lTime, const char* pszText) { 
	MIDIEvent* pEvent = MIDIEvent_CreateLyric (lTime, pszText);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�Ƀ}�[�J�[�C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertMarker (MIDITrack* pTrack, long lTime, const char* pszText) { 
	MIDIEvent* pEvent = MIDIEvent_CreateMarker (lTime, pszText);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�ɃL���[�|�C���g�C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertCuePoint (MIDITrack* pTrack, long lTime, const char* pszText) { 
	MIDIEvent* pEvent = MIDIEvent_CreateCuePoint (lTime, pszText);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�Ƀv���O�������C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertProgramName (MIDITrack* pTrack, long lTime, const char* pszText) { 
	MIDIEvent* pEvent = MIDIEvent_CreateProgramName (lTime, pszText);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�Ƀf�o�C�X���C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertDeviceName (MIDITrack* pTrack, long lTime, const char* pszText) { 
	MIDIEvent* pEvent = MIDIEvent_CreateDeviceName (lTime, pszText);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�Ƀ`�����l���v���t�B�b�N�X�C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertChannelPrefix (MIDITrack* pTrack, long lTime, long lCh) { 
	MIDIEvent* pEvent = MIDIEvent_CreateChannelPrefix (lTime, lCh);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�Ƀ|�[�g�v���t�B�b�N�X�C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertPortPrefix (MIDITrack* pTrack, long lTime, long lPort) { 
	MIDIEvent* pEvent = MIDIEvent_CreatePortPrefix (lTime, lPort);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�ɃG���h�I�u�g���b�N�C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertEndofTrack (MIDITrack* pTrack, long lTime) { 
	MIDIEvent* pEvent = MIDIEvent_CreateEndofTrack (lTime);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�Ƀe���|�C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertTempo (MIDITrack* pTrack, long lTime, long lTempo) { 
	MIDIEvent* pEvent = MIDIEvent_CreateTempo (lTime, lTempo);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N��SMPTE�I�t�Z�b�g�C�x���g�𐶐����đ}�� */
long MIDITrack_InsertSMPTEOffset 
(MIDITrack* pTrack, long lTime, long lMode, 
 long lHour, long lMin, long lSec, long lFrame, long lSubFrame) {
	MIDIEvent* pEvent = MIDIEvent_CreateSMPTEOffset 
		(lTime, lMode, lHour, lMin, lSec, lFrame, lSubFrame);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�ɔ��q�C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertTimeSignature 
(MIDITrack* pTrack, long lTime, long lnn, long ldd, long lcc, long lbb) { 
	MIDIEvent* pEvent = MIDIEvent_CreateTimeSignature (lTime, lnn, ldd, lcc, lbb);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�ɒ����C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertKeySignature (MIDITrack* pTrack, long lTime, long lsf, long lmi) { 
	MIDIEvent* pEvent = MIDIEvent_CreateKeySignature (lTime, lsf, lmi);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�ɃV�[�P���T�Ǝ��̃C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertSequencerSpecific (MIDITrack* pTrack, long lTime, char* pBuf, long lLen) { 
	MIDIEvent* pEvent = MIDIEvent_CreateSequencerSpecific (lTime, pBuf, lLen);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�Ƀm�[�g�I�t�C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertNoteOff
	(MIDITrack* pTrack, long lTime, long lCh, long lKey, long lVel) {
	MIDIEvent* pEvent = MIDIEvent_CreateNoteOff (lTime, lCh, lKey, lVel);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�Ƀm�[�g�I���C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertNoteOn
	(MIDITrack* pTrack, long lTime, long lCh, long lKey, long lVel) {
	MIDIEvent* pEvent = MIDIEvent_CreateNoteOn (lTime, lCh, lKey, lVel);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�Ƀm�[�g�C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertNote 
	(MIDITrack* pTrack, long lTime, long lCh, long lKey, long lVel, long lDur) {
	MIDIEvent* pEvent = MIDIEvent_CreateNote (lTime, lCh, lKey, lVel, lDur);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�ɃL�[�A�t�^�[�C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertKeyAftertouch
	(MIDITrack* pTrack, long lTime, long lCh, long lKey, long lVal) {
	MIDIEvent* pEvent = MIDIEvent_CreateKeyAftertouch (lTime, lCh, lKey, lVal);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�ɃR���g���[���[�C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertControlChange
	(MIDITrack* pTrack, long lTime, long lCh, long lNum, long lVal) {
	MIDIEvent* pEvent = MIDIEvent_CreateControlChange (lTime, lCh, lNum, lVal);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N��RPN�`�F���W�C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertRPNChange 
	(MIDITrack* pTrack, long lTime, long lCh, long lBank, long lNum) {
	MIDIEvent* pEvent = MIDIEvent_CreateRPNChange (lTime, lCh, lBank, lNum);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N��NRPN�`�F���W�C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertNRPNChange 
	(MIDITrack* pTrack, long lTime, long lCh, long lBank, long lNum) {
	MIDIEvent* pEvent = MIDIEvent_CreateNRPNChange (lTime, lCh, lBank, lNum);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�Ƀv���O�����C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertProgramChange
	(MIDITrack* pTrack, long lTime, long lCh, long lNum) {
	MIDIEvent* pEvent = MIDIEvent_CreateProgramChange (lTime, lCh, lNum);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�Ƀo���N�p�b�`�C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertPatchChange 
	(MIDITrack* pTrack, long lTime, long lCh, long lBank, long lNum) {
	MIDIEvent* pEvent = MIDIEvent_CreatePatchChange (lTime, lCh, lBank, lNum);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�Ƀ`�����l���A�t�^�[�C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertChannelAftertouch 
	(MIDITrack* pTrack, long lTime, long lCh, long lVal) {
	MIDIEvent* pEvent = MIDIEvent_CreateChannelAftertouch (lTime, lCh, lVal);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�Ƀs�b�`�x���h�C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertPitchBend
	(MIDITrack* pTrack, long lTime, long lCh, long lVal) {
	MIDIEvent* pEvent = MIDIEvent_CreatePitchBend (lTime, lCh, lVal);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N�ɃV�X�e���G�N�X�N���[�V�u�C�x���g�𐶐����đ}�� */
long __stdcall MIDITrack_InsertSysExEvent
	(MIDITrack* pTrack, long lTime, unsigned char* pBuf, long lLen) {
	MIDIEvent* pEvent = MIDIEvent_CreateSysExEvent (lTime, pBuf, lLen);
	if (pEvent == NULL) {
		return 0;
	}
	return MIDITrack_InsertEvent (pTrack, pEvent);
}

/* �g���b�N���̃C�x���g�𕡐����� */
long __stdcall MIDITrack_DuplicateEvent (MIDITrack* pTrack, MIDIEvent* pEvent) {
	long lCount = 0;
	MIDIEvent* pDuplicatedEvent = NULL;
	MIDIEvent* pTargetEvent = MIDIEvent_GetFirstCombinedEvent (pEvent);
	assert (pTrack);
	assert (pEvent);
	assert (pEvent->m_pParent == pTrack);
	pDuplicatedEvent = MIDIEvent_CreateClone (pTargetEvent);
	if (pDuplicatedEvent == NULL) {
		return 0;
	}
	while (pTargetEvent && pDuplicatedEvent) {
		MIDITrack_InsertSingleEventAfter (pTrack, pDuplicatedEvent, pTargetEvent);
		pTargetEvent = pTargetEvent->m_pNextCombinedEvent;
		pDuplicatedEvent = pDuplicatedEvent->m_pNextCombinedEvent;
		lCount++;
	}
	return lCount;
}

/* �g���b�N����C�x���g��1��菜��(�C�x���g�I�u�W�F�N�g�͍폜���܂���) */
long __stdcall MIDITrack_RemoveSingleEvent (MIDITrack* pTrack, MIDIEvent* pEvent) {
	assert (pTrack);
	assert (pEvent);
	assert (pEvent->m_pParent == pTrack);
	return MIDIEvent_SetFloating (pEvent);
}

/* �g���b�N����C�x���g����菜��(�C�x���g�I�u�W�F�N�g�͍폜���܂���) */
long __stdcall MIDITrack_RemoveEvent (MIDITrack* pTrack, MIDIEvent* pEvent) {
	long lCount = 0;
	MIDIEvent* pRemoveEvent = MIDIEvent_GetFirstCombinedEvent (pEvent);
	assert (pTrack);
	assert (pEvent);
	assert (pEvent->m_pParent == pTrack);
	if (pEvent->m_pParent != pTrack) {
		return 0;
	}
	while (pRemoveEvent) {
		MIDIEvent_SetFloating (pRemoveEvent);
		pRemoveEvent = pRemoveEvent->m_pNextCombinedEvent;
		lCount++;
	}
	return lCount;
}

/* MIDI�g���b�N�����V�g���b�N�ł��邩�ǂ����𒲂ׂ� */
long __stdcall MIDITrack_IsFloating (MIDITrack* pMIDITrack) {
	return (pMIDITrack->m_pParent == NULL ? 1 : 0);
}


/* MIDI�g���b�N���R���_�N�^�[�g���b�N�Ƃ��Đ��������Ƃ��m�F���� */
/* (�R���_�N�^�[�g���b�N�̏����́AMIDI�`�����l���C�x���g���܂܂Ȃ�����) */
long __stdcall MIDITrack_CheckSetupTrack (MIDITrack* pMIDITrack) {
	MIDIEvent* pMIDIEvent;
	forEachEvent (pMIDITrack, pMIDIEvent) {
		if (0x80 <= pMIDIEvent->m_lKind && pMIDIEvent->m_lKind <= 0xEF) {
			return 0;
		}
	}
	return 1;
}

/* MIDI�g���b�N���m���R���_�N�^�[�g���b�N�Ƃ��Đ��������Ƃ��m�F���� */
/* (�m���R���_�N�^�[�g���b�N�ɂ́A0x50��̃C�x���g(�e���|�ESMPTE�I�t�Z�b�g�E���q�L���E�����L���Ȃ�)�� */
/*�@�܂�ł͂Ȃ�Ȃ��B) */
long __stdcall MIDITrack_CheckNonSetupTrack (MIDITrack* pMIDITrack) {
	MIDIEvent* pMIDIEvent;
	forEachEvent (pMIDITrack, pMIDIEvent) {
		if (0x50 <= pMIDIEvent->m_lKind && pMIDIEvent->m_lKind < 0x60) {
			return 0;
		}
	}
	return 1;
}



/* �^�C���R�[�h���~���b�����ɕϊ�(�w��g���b�N���̃e���|�C�x���g����Ɍv�Z) */
long __stdcall MIDITrack_TimeToMillisec (MIDITrack* pMIDITrack, long lTime) {
	long lMode = 0;
	long lResolution = 0;
	assert (pMIDITrack);
	assert (lTime >= 0);
	/* ���V�g���b�N�͔F�߂Ȃ� */
	if (pMIDITrack->m_pParent == NULL) {
		return 0;
	}
	/* �^�C���x�[�X�𒲂ׂ� */
	MIDIData_GetTimeBase (((MIDIData*)(pMIDITrack->m_pParent)), &lMode, &lResolution);
	/* TPQN�x�[�X��MIDI�f�[�^�̏ꍇ */
	if (lMode == MIDIDATA_TPQNBASE) {
		long lDeltaTime = 0;
		long lOldTime = 0;
		long lOldTempo = MIDIEVENT_DEFTEMPO;
		int64_t llDeltaResoMicrosec = 0;
		int64_t llSumResoMicrosec = 0;
		/* �ŏ��̃e���|�C�x���g��T�� */
		MIDIEvent* pEvent = MIDITrack_GetFirstKindEvent (pMIDITrack, MIDIEVENT_TEMPO);
		/* �e���|�C�x���g�𔭌����邽�тɂ��̈ʒu�̃��]�ʕb�����߂� */
		while (pEvent) {
			if (pEvent->m_lTime > lTime) {
				break;
			}
			lDeltaTime = pEvent->m_lTime - lOldTime;
			llDeltaResoMicrosec = ((int64_t)lDeltaTime) * ((int64_t)lOldTempo);
			llSumResoMicrosec += llDeltaResoMicrosec;
			assert (llSumResoMicrosec >= 0);
			lOldTime = pEvent->m_lTime;
			lOldTempo = MIDIEvent_GetTempo (pEvent);
			pEvent = pEvent->m_pNextSameKindEvent;
		}
		/* �Ō�̃e���|�C�x���g����w��ʒu�܂ł̃��]�ʕb�����߂� */
		lDeltaTime = lTime - lOldTime;
		llDeltaResoMicrosec = ((int64_t)lDeltaTime) * ((int64_t)lOldTempo);
		llSumResoMicrosec += llDeltaResoMicrosec;
		assert (llSumResoMicrosec >= 0);
		llSumResoMicrosec /= lResolution;
		llSumResoMicrosec /= 1000;
		return (long)llSumResoMicrosec;
	}
	/* SMPTE�x�[�X��MIDI�f�[�^�̏ꍇ */
	else {
		int64_t llTemp;
		// lTime * 1000 / (lMode * lResolution);
		// lMode = MIDIDATA_SMPTE29BASE �� lMode = 29.97����;
		if (lMode == MIDIDATA_SMPTE29BASE) {
			llTemp = ((int64_t)lTime) * ((int64_t)100000);
			return (long)(llTemp / (int64_t)(2997 * lResolution));
		}
		llTemp = ((int64_t)lTime) * ((int64_t)1000);
		return (long)(llTemp / (int64_t)(lMode * lResolution));
	}
}

/* �~���b�������^�C���R�[�h�ɕϊ�(�w��g���b�N���̃e���|�C�x���g����Ɍv�Z) */
long __stdcall MIDITrack_MillisecToTime (MIDITrack* pMIDITrack, long lMillisec) {
	long lMode;
	long lResolution;
	assert (pMIDITrack);
	assert (lMillisec >= 0);
	/* ���V�g���b�N�͔F�߂Ȃ� */
	if (pMIDITrack->m_pParent == NULL) {
		return 0;
	}
	/* �^�C���x�[�X�𒲂ׂ� */
	MIDIData_GetTimeBase (((MIDIData*)(pMIDITrack->m_pParent)), &lMode, &lResolution);
	/* TPQN�x�[�X��MIDI�f�[�^�̏ꍇ */
	if (lMode == MIDIDATA_TPQNBASE) {
		long lDeltaTime = 0;
		long lOldTime = 0;
		long lOldTempo = MIDIEVENT_DEFTEMPO;
		int64_t llDeltaResoMicrosec = 0;
		int64_t llSumResoMicrosec = 0;
		int64_t llResoMicrosec = ((int64_t)lMillisec) * ((int64_t)lResolution) * ((int64_t)1000);
		/* �ŏ��̃e���|�C�x���g��T�� */
		MIDIEvent* pEvent = MIDITrack_GetFirstKindEvent (pMIDITrack, MIDIEVENT_TEMPO);
		/* �Ō�̃e���|�C�x���g�̃��]�ʕb��lSumResoMicrosec�Ɏ擾 */
		while (pEvent) {
			lDeltaTime = pEvent->m_lTime - lOldTime;
			llDeltaResoMicrosec = ((int64_t)lDeltaTime) * ((int64_t)lOldTempo);
			if (llSumResoMicrosec + llDeltaResoMicrosec > llResoMicrosec) {
				break;
			}
			llSumResoMicrosec += llDeltaResoMicrosec;
			lOldTime = pEvent->m_lTime;
			lOldTempo = MIDIEvent_GetTempo (pEvent);
			pEvent = pEvent->m_pNextSameKindEvent;
		}
		/* �Ō�̃e���|��������w�莞���܂ł̌o�߃^�C��[���]�}�C�N���b]���v�Z */
		llDeltaResoMicrosec = llResoMicrosec - llSumResoMicrosec;
		//lDeltaTime = (long)(llDeltaResoMicrosec / (int64_t)lResolution / (int64_t)lOldTempo);//20080920���C��
		lDeltaTime = (long)(llDeltaResoMicrosec / (int64_t)lOldTempo);
		return lOldTime + lDeltaTime;
	}
	/* SMPTE�x�[�X��MIDI�f�[�^�̏ꍇ */
	else {
		int64_t llTemp;
		if (lMode == MIDIDATA_SMPTE29BASE) {
			llTemp = (int64_t)2997 * (int64_t)lResolution * (int64_t)lMillisec;
			return (long)(llTemp / (int64_t)100000);
		}
		llTemp = (int64_t)lMode * (int64_t)lResolution * (int64_t)lMillisec;
		return (long)(llTemp / (int64_t)1000);
	}
}

/* TPQN������SMPTE�����ɕϊ�(�w��g���b�N���̃e���|�C�x���g����Ɍv�Z) */
/* ���̊֐���SetTimeBase��TPQN�x�[�X��SMPTE�x�[�X�ɕϊ����鎞�ɂ̂݌Ă΂��B */
long __stdcall MIDITrack_TPQNtoSMPTE (MIDITrack* pMIDITrack, long lTime, long lNewMode, long lNewResolution) {
	long lMode = 0;
	long lResolution = 0;
	assert (pMIDITrack);
	assert (lTime >= 0);
	/* ���V�g���b�N�͔F�߂Ȃ� */
	if (pMIDITrack->m_pParent == NULL) {
		return 0;
	}
	/* �^�C���x�[�X�𒲂ׂ� */
	MIDIData_GetTimeBase (((MIDIData*)(pMIDITrack->m_pParent)), &lMode, &lResolution);
	/* TPQN�x�[�X��MIDI�f�[�^�̏ꍇ */
	if (lMode == MIDIDATA_TPQNBASE) {
		long lDeltaTime = 0;
		long lOldTime = 0;
		long lOldTempo = MIDIEVENT_DEFTEMPO;
		int64_t llDeltaResoMicrosec = 0;
		int64_t llSumResoMicrosec = 0;
		int64_t llTemp = 0;
		/* �ŏ��̃e���|�C�x���g��T�� */
		MIDIEvent* pEvent = MIDITrack_GetFirstKindEvent (pMIDITrack, MIDIEVENT_TEMPO);
		/* �e���|�C�x���g�𔭌����邽�тɂ��̈ʒu�̃��]�ʕb�����߂� */
		while (pEvent) {
			if (pEvent->m_lTime > lTime) {
				break;
			}
			lDeltaTime = pEvent->m_lTime - lOldTime;
			llDeltaResoMicrosec = ((int64_t)lDeltaTime) * ((int64_t)lOldTempo);
			llSumResoMicrosec += llDeltaResoMicrosec;
			assert (llSumResoMicrosec >= 0);
			lOldTime = pEvent->m_lTime;
			lOldTempo = MIDIEvent_GetTempo (pEvent);
			pEvent = pEvent->m_pNextSameKindEvent;
		}
		/* �Ō�̃e���|�C�x���g����w��ʒu�܂ł̃��]�ʕb�����߂� */
		lDeltaTime = lTime - lOldTime;
		llDeltaResoMicrosec = ((int64_t)lDeltaTime) * ((int64_t)lOldTempo);
		llSumResoMicrosec += llDeltaResoMicrosec;
		assert (llSumResoMicrosec >= 0);
		if (lNewMode == MIDIDATA_SMPTE29BASE) {
			llTemp = llSumResoMicrosec * (int64_t)2997 * (int64_t)lNewResolution;
			return (long)(llTemp / 100000000 / (int64_t)lResolution);
		}
		llTemp = llSumResoMicrosec * (int64_t)lNewMode * (int64_t)lNewResolution;
		return (long)(llTemp / 1000000 / (int64_t)lResolution);
		
	}
	return 0;
}



/* SMPTE������TPQN�����ɕϊ�(�w��g���b�N���̃e���|�C�x���g����Ɍv�Z) */
/* ���̊֐���SetTimeBase��SMPTE�x�[�X��TPQN�x�[�X�ɕϊ����鎞�ɂ̂݌Ă΂��B */
long __stdcall MIDITrack_SMPTEtoTPQN (MIDITrack* pMIDITrack, long lTime, long lNewResolution) {
	long lMode;
	long lResolution;
	assert (pMIDITrack);
	assert (lTime >= 0);
	assert (lNewResolution > 0);
	/* ���V�g���b�N�͔F�߂Ȃ� */
	if (pMIDITrack->m_pParent == NULL) {
		return 0;
	}
	/* �^�C���x�[�X�𒲂ׂ� */
	MIDIData_GetTimeBase ((MIDIData*)(pMIDITrack->m_pParent), &lMode, &lResolution);
	/* SMPTE�x�[�X��MIDI�f�[�^�̏ꍇ */
	if (lMode != MIDIDATA_TPQNBASE) {
		long lDeltaTime = 0;
		long lOldTime = 0;
		long lOldTempo = MIDIEVENT_DEFTEMPO;
		int64_t llDeltaResoModeTick = 0;
		int64_t llSumResoModeTick = 0;
		/* �ŏ��̃e���|�C�x���g��T�� */
		MIDIEvent* pEvent = MIDITrack_GetFirstKindEvent (pMIDITrack, MIDIEVENT_TEMPO);
		/* �Ō�̃e���|�C�x���g�̈ʒu[���]���[�h�e�B�b�N]��lSumResoModeTick�Ɏ擾 */
		while (pEvent) {
			if (pEvent->m_lTime >= lTime) {
				break;
			}
			lDeltaTime = pEvent->m_lTime - lOldTime; /* [SMPTEframe=���]���[�h�b] */
			llDeltaResoModeTick = ((int64_t)lDeltaTime * (int64_t)1000000 * (int64_t)lNewResolution / (int64_t)lOldTempo);
			llSumResoModeTick += llDeltaResoModeTick;
			assert (llSumResoModeTick >= 0);
			lOldTime = pEvent->m_lTime;
			lOldTempo = MIDIEvent_GetTempo (pEvent);
			pEvent = pEvent->m_pNextSameKindEvent;
		}
		/* �Ō�̃e���|��������w�莞���܂ł̌o�߃^�C�����v�Z */
		lDeltaTime = lTime - lOldTime; /* [SMPTEframe=���]���[�h�b] */
		llDeltaResoModeTick = ((int64_t)lDeltaTime * (int64_t)1000000 * (int64_t)lNewResolution / (int64_t)lOldTempo);
		llSumResoModeTick += llDeltaResoModeTick;
		assert (llSumResoModeTick >= 0);
		if (lMode == MIDIDATA_SMPTE29BASE) {
			return (long)(llSumResoModeTick * (int64_t)100 / (int64_t)(2997 * lResolution));
		}
		//llSumResoModeTick /= lMode;
		//llSumResoModeTick /= lResolution;
		//return (long)llSumResoModeTick;
		return (long)(llSumResoModeTick / (int64_t)(lMode * lResolution));
	}
	return 0;
}




/* �^�C���R�[�h�����߁F���F�e�B�b�N�ɕ���(�w��g���b�N���̔��q�L������v�Z) */
long __stdcall MIDITrack_BreakTimeEx
(MIDITrack* pMIDITrack, long lTime, long* pMeasure, long* pBeat, long* pTick,
 long* pnn, long* pdd, long* pcc, long* pbb) {
	long lOldTime = 0;
	long lOldnn = 4;
	long lOlddd = 2;
	long lOldcc = 24;
	long lOldbb = 8;
	long lnn, ldd, lcc, lbb;
	long lSumMeasure = 0;
	long lDeltaMeasure;
	long lDeltaTime;
	long lUnitTick;
	long lTimeMode;
	long lTimeResolution;
	MIDIData* pMIDIData = (MIDIData*)(pMIDITrack->m_pParent);
	MIDIEvent* pMIDIEvent;
	/* ���V�g���b�N�͔F�߂Ȃ� */
	if (pMIDIData == NULL) {
		return 0;
	}
	/* MIDI�f�[�^�̃^�C�����[�h�ƕ���\���擾 */
	MIDIData_GetTimeBase (pMIDIData, &lTimeMode, &lTimeResolution);

	/* TPQN�x�[�X��MIDI�f�[�^�̏ꍇ */
	if (lTimeMode == MIDIDATA_TPQNBASE) {
		/* �ŏ��̔��q�L�����擾 */
		pMIDIEvent = MIDITrack_GetFirstKindEvent (pMIDITrack, MIDIEVENT_TIMESIGNATURE);
		/* ���q�L���𔭌����邽�тɂ��̈ʒu�̏��ߔԍ������߂� */
		while (pMIDIEvent) {
			if (pMIDIEvent->m_lTime >= lTime) {
				break;
			}
			MIDIEvent_GetTimeSignature (pMIDIEvent, &lnn, &ldd, &lcc, &lbb);
			lDeltaTime = pMIDIEvent->m_lTime - lOldTime;
			lUnitTick = lTimeResolution * 4 / (1 << lOlddd); /* ���q1������̃e�B�b�N�� */
			if (lDeltaTime > 0) {
				lDeltaMeasure = (lDeltaTime - 1) / (lUnitTick * lOldnn) + 1;
			}
			else {
				lDeltaMeasure = 0;
			}
			lSumMeasure += lDeltaMeasure;
			lOldTime = pMIDIEvent->m_lTime;
			lOldnn = lnn;
			lOlddd = ldd;
			lOldcc = lcc;
			lOldbb = lbb;
			pMIDIEvent = pMIDIEvent->m_pNextSameKindEvent;
		}
		/* �Ō�̔��q�L������w��ʒu�܂ł̑��������߂� */
		lDeltaTime = lTime - lOldTime;
		lUnitTick = lTimeResolution * 4 / (1 << lOlddd); /* ���q1������̃e�B�b�N�� */
		if (lDeltaTime >= 0) {
			lDeltaMeasure = (lDeltaTime) / (lUnitTick * lOldnn);
		}
		else {
			lDeltaMeasure = 0;
		}
		*pMeasure = lSumMeasure + lDeltaMeasure;
		*pBeat = (lDeltaTime % (lUnitTick * lOldnn)) / lUnitTick;
		*pTick = lDeltaTime % lUnitTick;
		*pnn = lOldnn;
		*pdd = lOlddd;
		*pcc = lOldcc;
		*pbb = lOldbb;
	}
	/* SMPTE�x�[�X��MIDI�f�[�^�̏ꍇ */
	else {
		*pMeasure = lTime / lTimeResolution;
		*pBeat = 0;
		*pTick = lTime % lTimeResolution;
	}
	return 1;
}

/* �^�C���R�[�h�����߁F���F�e�B�b�N�ɕ���(�w��g���b�N���̔��q�L������v�Z) */
long __stdcall MIDITrack_BreakTime 
(MIDITrack* pMIDITrack, long lTime, long* pMeasure, long* pBeat, long* pTick) {
	long lnn, lbb, lcc, ldd;
	return MIDITrack_BreakTimeEx (pMIDITrack, lTime, pMeasure, pBeat, pTick, 
		&lnn, &lbb, &lcc, &ldd);
}

/* ���߁F���F�e�B�b�N����^�C���R�[�h�𐶐�(�w��g���b�N���̔��q�L������v�Z) */
long __stdcall MIDITrack_MakeTimeEx 
(MIDITrack* pMIDITrack, long lMeasure, long lBeat, long lTick, 
 long* pTime, long* pnn, long* pdd, long* pcc, long* pbb) {
	long lOldTime = 0;
	long lOldnn = 4;
	long lOlddd = 2;
	long lOldcc = 24;
	long lOldbb = 8;
	long lnn, ldd, lcc, lbb;
	long lSumMeasure = 0;
	long lDeltaMeasure;
	long lDeltaTime;
	long lUnitTick;
	long lTimeMode;
	long lTimeResolution;
	MIDIData* pMIDIData = (MIDIData*)(pMIDITrack->m_pParent);
	MIDIEvent* pMIDIEvent;
	/* ���V�g���b�N�͔F�߂Ȃ� */
	if (pMIDIData == NULL) {
		return 0;
	}
	/* �^�C�����[�h�ƕ���\���擾 */
	MIDIData_GetTimeBase (pMIDIData, &lTimeMode, &lTimeResolution);
	/* TPQN�x�[�X��MIDI�f�[�^�̏ꍇ */
	if (lTimeMode == MIDIDATA_TPQNBASE) {
		/* �ŏ��̔��q�L���C�x���g���擾 */
		pMIDIEvent = MIDITrack_GetFirstKindEvent (pMIDITrack, MIDIEVENT_TIMESIGNATURE);
		/* ���q�L���𔭌����邽�тɂ��̈ʒu�̏��ߔԍ��𒲂ׂ� */
		while (pMIDIEvent) {
			MIDIEvent_GetTimeSignature (pMIDIEvent, &lnn, &ldd, &lcc, &lbb);
			lDeltaTime = pMIDIEvent->m_lTime - lOldTime;
			lUnitTick = lTimeResolution * 4 / (1 << lOlddd); /* ���q1������̃e�B�b�N�� */
			if (lDeltaTime > 0) {
				lDeltaMeasure = (lDeltaTime - 1) / (lUnitTick * lOldnn) + 1;
			}
			else {
				lDeltaMeasure = 0;
			}
			if (lSumMeasure + lDeltaMeasure > lMeasure) {
				break;
			}
			lSumMeasure += lDeltaMeasure;
			lOldTime = pMIDIEvent->m_lTime;
			lOldnn = lnn;
			lOlddd = ldd;
			lOldcc = lcc;
			lOlddd = ldd;
			pMIDIEvent = pMIDIEvent->m_pNextSameKindEvent;
		}
		/* �Ō�̔��q�L������̌o�߃e�B�b�N�������Z���� */
		lUnitTick = lTimeResolution * 4 / (1 << lOlddd);
		lDeltaMeasure = lMeasure - lSumMeasure;
		*pTime = lOldTime + lUnitTick * lOldnn * lDeltaMeasure + lUnitTick * lBeat+ lTick; 
	
		/* �w��ʒu�̔��q�����i�[ */
		*pnn = lOldnn;
		*pdd = lOlddd;
		*pcc = lOldcc;
		*pbb = lOldbb;
	}
	/* SMPTE�x�[�X��MIDI�f�[�^�̏ꍇ */
	else {
		*pTime = lMeasure * lTimeResolution + lTick;
	}
	return 1;
}

/* ���߁F���F�e�B�b�N����^�C���R�[�h�𐶐�(�w��g���b�N���̔��q�L������v�Z) */
long __stdcall MIDITrack_MakeTime
(MIDITrack* pMIDITrack, long lMeasure, long lBeat, long lTick, long* pTime) {
	long lnn, lbb, lcc, ldd;
	return MIDITrack_MakeTimeEx (pMIDITrack, lMeasure, lBeat, lTick, pTime, 
		&lnn, &lbb, &lcc, &ldd);
}

/* �w��ʒu�ɂ����钼�O�̃e���|���擾 */
long __stdcall MIDITrack_FindTempo
(MIDITrack* pMIDITrack, long lTime, long* pTempo) {
	long lTempo = MIDIEVENT_DEFTEMPO;
	MIDIEvent* pMIDIEvent = NULL;
	pMIDIEvent = MIDITrack_GetFirstKindEvent (pMIDITrack, MIDIEVENT_TEMPO);
	while (pMIDIEvent) {
		if (pMIDIEvent->m_lTime <= lTime) {
			lTempo = MIDIEvent_GetTempo (pMIDIEvent);
		}
		else {
			break;
		}
		pMIDIEvent = MIDIEvent_GetNextSameKindEvent (pMIDIEvent);
	}
	*pTempo = lTempo;
	return 1;
}

/* �w��ʒu�ɂ����钼�O�̔��q�L�����擾 */
long __stdcall MIDITrack_FindTimeSignature 
(MIDITrack* pMIDITrack, long lTime, long* pnn, long* pdd, long* pcc, long* pbb) {
	long lnn = 4;
	long ldd = 2;
	long lcc = 24;
	long lbb = 8;
	MIDIEvent* pMIDIEvent = NULL;
	pMIDIEvent = MIDITrack_GetFirstKindEvent (pMIDITrack, MIDIEVENT_TIMESIGNATURE);
	while (pMIDIEvent) {
		if (pMIDIEvent->m_lTime <= lTime) {
			MIDIEvent_GetTimeSignature (pMIDIEvent, &lnn, &ldd, &lcc, &lbb);
		}
		else {
			break;
		}
		pMIDIEvent = MIDIEvent_GetNextSameKindEvent (pMIDIEvent);
	}
	*pnn = lnn;
	*pdd = ldd;
	*pcc = lcc;
	*pbb = lbb;
	return 1;
}

/* �w��ʒu�ɂ����钼�O�̒����L�����擾 */
long __stdcall MIDITrack_FindKeySignature 
(MIDITrack* pMIDITrack, long lTime, long* psf, long* pmi) {
	long lsf = 0;
	long lmi = 0;
	MIDIEvent* pMIDIEvent = NULL;
	pMIDIEvent = MIDITrack_GetFirstKindEvent (pMIDITrack, MIDIEVENT_KEYSIGNATURE);
	while (pMIDIEvent) {
		if (pMIDIEvent->m_lTime <= lTime) {
			MIDIEvent_GetKeySignature (pMIDIEvent, &lsf, &lmi);
		}
		else {
			break;
		}
		pMIDIEvent = MIDIEvent_GetNextSameKindEvent (pMIDIEvent);
	}
	*psf = lsf;
	*pmi = lmi;
	return 1;
}







/******************************************************************************/
/*                                                                            */
/*�@MIDIData�N���X�֐��Q                                                      */
/*                                                                            */
/******************************************************************************/


/* �g���b�N��pTarget�̒��O�ɑ}��(�g���b�N�͗\�ߐ������Ă���) */
long __stdcall MIDIData_InsertTrackBefore 
(MIDIData* pMIDIData, MIDITrack* pTrack, MIDITrack* pTarget) {
	assert (pMIDIData);
	assert (pTrack);
	/* �g���b�N������MIDI�f�[�^�ɑ����Ă���ꍇ�͒ǉ��ł��Ȃ��B */
	if (pTrack->m_pPrevTrack || pTrack->m_pNextTrack || pTrack->m_pParent) {
		return 0;
	}
	/* �^�[�Q�b�g�g���b�N�̐e��MIDI�f�[�^����v���Ȃ��ꍇ�͋p���B */
	if (pTarget) {
		if (pTarget->m_pParent != pMIDIData) {
			return 0;
		}
	}
	/* SMF0�ɂ�����}���������`�F�b�N */
	if (pMIDIData->m_lFormat == 0) {
		/* �ʏ��MIDI�f�[�^�̏ꍇ */
		if (MIDIData_GetXFVersion (pMIDIData) == 0 && MIDITrack_GetXFVersion (pTrack) == 0) {
			/* ����1�ȏ�̃g���b�N�������Ă���ꍇ�A����ȏ�ǉ��ł��Ȃ� */
			if (pMIDIData->m_pFirstTrack || pMIDIData->m_pLastTrack) {
				return 0;
			}
		}
		/* XF�f�[�^�̏ꍇ�͑}���ł��� */
	}
	/* SMF1�ɂ�����}���������`�F�b�N��(�R���_�N�^�[�g���b�N�̔���Ȃ�)���܂��� */
	/* pTarget�̒��O�ɑ}������ꍇ */
	if (pTarget) {
		pTrack->m_pNextTrack = pTarget;
		pTrack->m_pPrevTrack = pTarget->m_pPrevTrack;
		pTrack->m_pParent = pMIDIData;
		if (pTarget->m_pPrevTrack) {
			pTarget->m_pPrevTrack->m_pNextTrack = pTrack;
		}
		else {
			pMIDIData->m_pFirstTrack = pTrack;
		}
		pTarget->m_pPrevTrack = pTrack;
	}
	/* �Ō�ɑ}������ꍇ(pData==NULL) */
	else if (pMIDIData->m_pLastTrack) {
		pTrack->m_pNextTrack = NULL;
		pTrack->m_pPrevTrack = pMIDIData->m_pLastTrack;
		pTrack->m_pParent = pMIDIData;
		pMIDIData->m_pLastTrack->m_pNextTrack = pTrack;
		pMIDIData->m_pLastTrack = pTrack;
	}
	/* ��̃g���b�N�ɑ}������ꍇ */
	else {
		pTrack->m_pNextTrack = NULL;
		pTrack->m_pPrevTrack = NULL;
		pTrack->m_pParent = pMIDIData;
		pMIDIData->m_pFirstTrack = pTrack;
		pMIDIData->m_pLastTrack = pTrack;
	}
	pMIDIData->m_lNumTrack++;
	return 1;
}


/* �g���b�N��pTarget�̒���ɑ}��(�g���b�N�͗\�ߐ������Ă���) */
long __stdcall MIDIData_InsertTrackAfter
(MIDIData* pMIDIData, MIDITrack* pTrack, MIDITrack* pTarget) {
	assert (pMIDIData);
	assert (pTrack);
	/* �g���b�N������MIDI�f�[�^�ɑ����Ă���ꍇ�͒ǉ��ł��Ȃ��B */
	if (pTrack->m_pPrevTrack || pTrack->m_pNextTrack || pTrack->m_pParent) {
		return 0;
	}
	/* �^�[�Q�b�g�g���b�N�̐e��MIDI�f�[�^����v���Ȃ��ꍇ�͋p���B */
	if (pTarget) {
		if (pTarget->m_pParent != pMIDIData) {
			return 0;
		}
	}
	/* SMF0�ɂ�����}���������`�F�b�N */
	if (pMIDIData->m_lFormat == 0) {
		/* �ʏ��MIDI�f�[�^�̏ꍇ */
		if (MIDIData_GetXFVersion (pMIDIData) == 0 && MIDITrack_GetXFVersion (pTrack) == 0) {
			/* ����1�ȏ�̃g���b�N�������Ă���ꍇ�A����ȏ�ǉ��ł��Ȃ� */
			if (pMIDIData->m_pFirstTrack || pMIDIData->m_pLastTrack) {
				return 0;
			}
		}
		/* XF�f�[�^�̏ꍇ�͑}���ł��� */
	}
	/* SMF1�ɂ�����}���������`�F�b�N��(�R���_�N�^�[�g���b�N�̔���Ȃ�)���܂��� */
	/* pTarget�̒���ɑ}������ꍇ */
	if (pTarget) {
		pTrack->m_pNextTrack = pTarget->m_pNextTrack;
		pTrack->m_pPrevTrack = pTarget;
		pTrack->m_pParent = pMIDIData;
		if (pTarget->m_pNextTrack) {
			pTarget->m_pNextTrack->m_pPrevTrack = pTrack;
		}
		else {
			pMIDIData->m_pLastTrack = pTrack;
		}
		pTarget->m_pNextTrack = pTrack;
	}
	/* �ŏ��ɑ}������ꍇ(pData==NULL) */
	else if (pMIDIData->m_pFirstTrack) {
		pTrack->m_pNextTrack = pMIDIData->m_pFirstTrack;
		pTrack->m_pPrevTrack = NULL;
		pTrack->m_pParent = pMIDIData;
		pMIDIData->m_pFirstTrack->m_pPrevTrack = pTrack;
		pMIDIData->m_pFirstTrack = pTrack;
	}
	/* ��̃g���b�N�ɑ}������ꍇ */
	else {
		pTrack->m_pNextTrack = NULL;
		pTrack->m_pPrevTrack = NULL;
		pTrack->m_pParent = pMIDIData;
		pMIDIData->m_pFirstTrack = pTrack;
		pMIDIData->m_pLastTrack = pTrack;
	}
	pMIDIData->m_lNumTrack++;
	return 1;
}

/* MIDI�f�[�^�Ƀg���b�N��ǉ�(�g���b�N�͗\�ߐ������Ă���) */
long __stdcall MIDIData_AddTrack (MIDIData* pMIDIData, MIDITrack* pTrack) {
	assert (pMIDIData != NULL);
	assert (pTrack != NULL);
	return MIDIData_InsertTrackAfter (pMIDIData, pTrack, pMIDIData->m_pLastTrack);
}

/* MIDI�f�[�^�ɋ����I�Ƀg���b�N��ǉ�(�g���b�N�͗\�ߐ������Ă���)(����J�֐�) */
long __stdcall MIDIData_AddTrackForce (MIDIData* pMIDIData, MIDITrack* pTrack) {
	assert (pMIDIData != NULL);
	assert (pTrack != NULL);
	if (pTrack->m_pPrevTrack != NULL ||
		pTrack->m_pNextTrack != NULL ||
		pTrack->m_pParent != NULL) {
		return 0;
	}
	pTrack->m_pNextTrack = NULL;
	pTrack->m_pPrevTrack = pMIDIData->m_pLastTrack;
	pTrack->m_pParent = pMIDIData;
	if (pMIDIData->m_pLastTrack) {
		pMIDIData->m_pLastTrack->m_pNextTrack = pTrack;
	}
	else {
		pMIDIData->m_pFirstTrack = pTrack;
	}
	pMIDIData->m_pLastTrack = pTrack;
	pMIDIData->m_lNumTrack++;
	return 1;
}

/* MIDI�f�[�^���̃g���b�N�𕡐����� */
long __stdcall MIDIData_DuplicateTrack (MIDIData* pMIDIData, MIDITrack* pTrack) {
	long lCount = 0;
	MIDITrack* pDuplicatedTrack = NULL;
	assert (pMIDIData);
	assert (pTrack);
	assert (pTrack->m_pParent == pMIDIData);
	/* SMF�t�H�[�}�b�g0�̏ꍇ�̓g���b�N�̕����͂ł��Ȃ� */
	if (pMIDIData->m_lFormat == 0) {
		return 0;
	}
	/* SMF�t�H�[�}�b�g1�̍ŏ��̃g���b�N�͕����ł��Ȃ� */
	if (pMIDIData->m_lFormat == 1 && pTrack == pMIDIData->m_pFirstTrack) {
		return 0;
	}
	/* �g���b�N�̕��� */
	pDuplicatedTrack = MIDITrack_CreateClone (pTrack);
	if (pDuplicatedTrack == NULL) {
		return 0;
	}
	MIDIData_InsertTrackAfter (pMIDIData, pDuplicatedTrack, pTrack);
	return 1;
}


/* MIDIData���̃g���b�N������(�g���b�N���̋y�уC�x���g�͍폜���Ȃ�) */
long __stdcall MIDIData_RemoveTrack (MIDIData* pMIDIData, MIDITrack* pTrack) {
	assert (pMIDIData);
	assert (pTrack);
	if (pTrack->m_pParent != pMIDIData) {
		return 0;
	}
	if (pTrack->m_pNextTrack) {
		pTrack->m_pNextTrack->m_pPrevTrack = pTrack->m_pPrevTrack;
	}
	else if (pMIDIData->m_pLastTrack) {
		pMIDIData->m_pLastTrack = pTrack->m_pPrevTrack;
	}
	if (pTrack->m_pPrevTrack) {
		pTrack->m_pPrevTrack->m_pNextTrack = pTrack->m_pNextTrack;
	}
	else if (pMIDIData->m_pFirstTrack) {
		pMIDIData->m_pFirstTrack = pTrack->m_pNextTrack;
	}
	pTrack->m_pNextTrack = NULL;
	pTrack->m_pPrevTrack = NULL;
	pTrack->m_pParent = NULL;
	pMIDIData->m_lNumTrack--;
	return 1;
}

/* MIDI�f�[�^�̍폜(�g���b�N���̃C�x���g����Ƃ��Ռ`���Ȃ�) */
void __stdcall MIDIData_Delete (MIDIData* pMIDIData) {
	MIDITrack* pCurTrack;	
	MIDITrack* pNextTrack;
	if (pMIDIData == NULL) {
		return;
	}
	pCurTrack = pMIDIData->m_pFirstTrack;
	while (pCurTrack) {
		pNextTrack = pCurTrack->m_pNextTrack;
		MIDITrack_Delete (pCurTrack);
		pCurTrack = pNextTrack;
	}
	free (pMIDIData);
	return;
}

/* MIDI�f�[�^�𐶐����AMIDI�f�[�^�ւ̃|�C���^��Ԃ�(���s��NULL) */
MIDIData* __stdcall MIDIData_Create (long lFormat, long lNumTrack, long lTimeMode, long lResolution) {
	MIDIData* pMIDIData;
	MIDITrack* pTrack;
	long i;

	/* �^�C�����[�h�ƕ���\�̐������`�F�b�N */
	if (lTimeMode == MIDIDATA_TPQNBASE) {
		if (lResolution < MIDIDATA_MINTPQNRESOLUTION || lResolution > MIDIDATA_MAXTPQNRESOLUTION) {
			return NULL;
		}
	}
	else if (lTimeMode == MIDIDATA_SMPTE24BASE || lTimeMode == MIDIDATA_SMPTE25BASE || 
		lTimeMode == MIDIDATA_SMPTE29BASE || lTimeMode == MIDIDATA_SMPTE30BASE) {
		if (lResolution < MIDIDATA_MINSMPTERESOLUTION || lResolution > MIDIDATA_MAXSMPTERESOLUTION) {
			return NULL;
		}
	}
	else {
		return NULL;
	}

	/* �t�H�[�}�b�g�ƃg���b�N���̐������`�F�b�N */
	if (lFormat == 0) {
		if (lNumTrack < 0 || lNumTrack >= 2) {
			return NULL;
		}
	}
	else if (lFormat == 1 || lFormat == 2) {
		if (lNumTrack < 0) {
			return NULL;
		}
	}
	else {
		return NULL;
	}
	
	pMIDIData = malloc (sizeof(MIDIData));
	if (pMIDIData == NULL) {
		return NULL;
	}
	pMIDIData->m_lFormat = lFormat;
	pMIDIData->m_lNumTrack = 0;
	if (lTimeMode == MIDIDATA_TPQNBASE) {
		pMIDIData->m_lTimeBase = lResolution;
	}
	else {
		pMIDIData->m_lTimeBase = ((256 - lTimeMode) << 8) | lResolution;
	}
	pMIDIData->m_pFirstTrack = NULL;
	pMIDIData->m_pLastTrack = NULL;
	pMIDIData->m_pNextSeq = NULL;
	pMIDIData->m_pPrevSeq = NULL;
	pMIDIData->m_pParent = NULL;
	pMIDIData->m_lReserved1 = 0;
	pMIDIData->m_lReserved2 = 0;
	pMIDIData->m_lReserved3 = 0;
	pMIDIData->m_lReserved4 = 0;
	pMIDIData->m_lUser1 = 0;
	pMIDIData->m_lUser2 = 0;
	pMIDIData->m_lUser3 = 0;
	for (i = 0; i < lNumTrack; i++) {
		pTrack = MIDITrack_Create ();
		if (pTrack == NULL) {
			break;
		}
		MIDIData_AddTrackForce (pMIDIData, pTrack);
	}
	if (i < lNumTrack) {
		MIDIData_Delete (pMIDIData);
		return NULL;
	}
	return pMIDIData;
}

/* MIDI�f�[�^�̊e�g���b�N�̏o�̓|�[�g�������ݒ肷��(����J) */
long __stdcall MIDIData_UpdateOutputPort (MIDIData* pMIDIData) {
	MIDITrack* pMIDITrack = NULL;
	MIDIEvent* pMIDIEvent = NULL;
	assert (pMIDIData);
	forEachTrack (pMIDIData, pMIDITrack) {
		/* �|�[�g�v���t�B�b�N�X�C�x���g�ŏo�̓|�[�g���w�肳��Ă���ꍇ�A */
		/* ���̃g���b�N�̏o�̓|�[�g�́A���̃|�[�g�ԍ��Ƃ��� */
		/* �������|�[�g�v���t�B�b�N�X�̒l&0x0F(0�`15)���̗p�l�Ƃ���B */
		/* �|�[�g�v���t�B�b�N�X�C�x���g���Ȃ��ꍇ�A */
		/* ���̃g���b�N�̏o�̓|�[�g��0��ݒ肷��B */
		MIDITrack_SetOutputPort (pMIDITrack, 0);
		forEachEvent (pMIDITrack, pMIDIEvent) {
			if (MIDIEvent_IsPortPrefix (pMIDIEvent)) {
				long lValue = MIDIEvent_GetValue (pMIDIEvent);
				MIDITrack_SetOutputPort (pMIDITrack, (lValue));
				break;
			}
		}
	}
	return 1;
}

/* MIDI�f�[�^�̊e�g���b�N�̏o�̓`�����l���������ݒ肷��(����J) */
long __stdcall MIDIData_UpdateOutputChannel (MIDIData* pMIDIData) {
	MIDITrack* pMIDITrack = NULL;
	MIDIEvent* pMIDIEvent = NULL;
	assert (pMIDIData);
	forEachTrack (pMIDIData, pMIDITrack) {
		/* ���̃g���b�N���̑SMIDI�`�����l���C�x���g�̃`�����l���ԍ�����v���Ă���ꍇ�A */
		/* ���̃g���b�N�̏o�̓`�����l���́A���̃`�����l���ԍ��Ƃ���B */
		/* ���̃g���b�N����MIDI�`�����l���C�x���g�̃`�����l���ԍ��� ���܂��܂ł���ꍇ�A */
		/* ���̃g���b�N�̏o�̓`�����l���́An/a�Ƃ���B */
		/* ���̃g���b�N����MIDI�`�����l���C�x���g���Ȃ��ꍇ�A */
		/* ���̃g���b�N�̏o�̓`�����l���́An/a�Ƃ���B */
		long lChannelChanged = -1;
		long lNewChannel = -1;
		long lOldChannel = -1;
		forEachEvent (pMIDITrack, pMIDIEvent) {
			if (MIDIEvent_IsMIDIEvent (pMIDIEvent)) {
				lNewChannel = MIDIEvent_GetChannel (pMIDIEvent);
				assert (0 <= lNewChannel && lNewChannel < 16);
				if (lOldChannel != lNewChannel) {
					lChannelChanged ++;
				}
				// 2��ވȏ�̃`�����l����MIDI�`�����l���C�x���g���������Ă���
				if (lChannelChanged >= 1) {
					MIDITrack_SetOutputChannel (pMIDITrack, -1);
					break;
				}
				lOldChannel = lNewChannel;
			}
		}
		// 1��ނ̃`�����l����MIDI�`�����l���C�x���g�݂̂ō\������Ă���
		if (lChannelChanged <= 0) {
			MIDITrack_SetOutputChannel (pMIDITrack, lNewChannel);
		}
	}
	return 1;
}

#define MIDITRACK_MODENATIVE          0x00000000 /* Native */
#define MIDITRACK_MODEGM1             0x7E000001 /* GM1 */
#define MIDITRACK_MODEGMOFF           0x7E000002 /* GMOff(=Native) */
#define MIDITRACK_MODEGM2             0x7E000003 /* GM2 */
#define MIDITRACK_MODEGS              0x41000002 /* GS */
#define MIDITRACK_MODE88              0x41000003 /* 88 */
#define MIDITRACK_MODEXG              0x43000002 /* XG */

/* MIDI�f�[�^�̊e�g���b�N�̕\�����[�h���u�ʏ�v���u�h�����v���������ݒ肷��(����J) */
long __stdcall MIDIData_UpdateViewMode (MIDIData* pMIDIData) {
	/* �f�o�C�XID�͂Ȃ�ł��悢���̂Ƃ���B */
	unsigned char byGMReset[] = 
		{0xF0, 0x7E, 0x7F, 0x09, 0x01, 0xF7};
	unsigned char byGMOff[] = 
		{0xF0, 0x7E, 0x7F, 0x09, 0x02, 0xF7};
	unsigned char byGM2Reset[] = 
		{0xF0, 0x7E, 0x7F, 0x09, 0x03, 0xF7};
	unsigned char byGSReset[] =
		{0xF0, 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41, 0xF7};
	unsigned char by88Reset[] =
		{0xF0, 0x41, 0x10, 0x42, 0x12, 0x00, 0x00, 0x7F, 0x00, 0x01, 0xF7};
	unsigned char byXGReset[] = 
		{0xF0, 0x43, 0x10, 0x4C, 0x00, 0x00, 0x7E, 0x00, 0xF7};
	
	MIDITrack* pTempTrack = NULL;
	MIDIEvent* pTempEvent = NULL;
	long lOutputPortMode[MIDIDATA_MAXNUMPORT];
	long lOutputChannelDrum[MIDIDATA_MAXNUMPORT][16];
	memset (lOutputPortMode, 0, sizeof(long) * MIDIDATA_MAXNUMPORT);
	memset (lOutputChannelDrum, 0, sizeof(long) * MIDIDATA_MAXNUMPORT * 16);
	assert (pMIDIData);

	/* �e�o�̓|�[�g�̉������[�h(Native/GM/GS/XG/GM2)���擾���� */
	forEachTrack (pMIDIData, pTempTrack) {
		long lTempPort = CLIP (0, MIDITrack_GetOutputPort (pTempTrack), MIDIDATA_MAXNUMPORT - 1);
		long lTempChannel = CLIP (-1, MIDITrack_GetOutputChannel (pTempTrack), 15);
		forEachEvent (pTempTrack, pTempEvent) {
			if (MIDIEvent_IsSysExEvent (pTempEvent)) {
				long lLen = MIDIEvent_GetLen (pTempEvent);
				unsigned char bySysxData[256];
				memset (bySysxData, 0, sizeof (bySysxData));
				MIDIEvent_GetData (pTempEvent, bySysxData, MIN (lLen, 255));
				if (memcmp (bySysxData, byGMReset, 6) == 0) {
					lOutputPortMode[lTempPort] = MIDITRACK_MODEGM1;
					break;
				}
				if (memcmp (bySysxData, byGMOff, 6) == 0) {
					lOutputPortMode[lTempPort] = MIDITRACK_MODEGMOFF;
					break;
				}
				if (memcmp (bySysxData, byGM2Reset, 6) == 0) {
					lOutputPortMode[lTempPort] = MIDITRACK_MODEGM2;
					break;
				}
				if (memcmp (bySysxData, byGSReset, 11) == 0) {
					lOutputPortMode[lTempPort] = MIDITRACK_MODEGS;
					break;
				}
				if (memcmp (bySysxData, by88Reset, 11) == 0) {
					lOutputPortMode[lTempPort] = MIDITRACK_MODE88;
					break;
				}
				if (memcmp (bySysxData, byXGReset, 9) == 0) {
					lOutputPortMode[lTempPort] = MIDITRACK_MODEXG;
					break;
				}
				/* TODO:�����ɐV�^�����̃��Z�b�g���b�Z�[�W�֑Ή����Ă��������B */
			}
		}
	}

	/* �o�̓|�[�g�̃`�����l�����Ƃɒʏ�(0)���h����(1)���̕��ނ��s���B*/
	forEachTrack (pMIDIData, pTempTrack) {
		long lTempPort = CLIP (0, MIDITrack_GetOutputPort (pTempTrack), MIDIDATA_MAXNUMPORT - 1);
		long lTempChannel = CLIP (-1, MIDITrack_GetOutputChannel (pTempTrack), 15);
		long lTempPortMode = lOutputPortMode[lTempPort];
		/* ���̃g���b�N�̏o�̓|�[�g�̉������������[�h�� */
		switch (lTempPortMode) {
		/* GM2�̏ꍇ */
		case MIDITRACK_MODEGM2:
			forEachEvent (pTempTrack, pTempEvent) {
				/* CC#0(Bank Select MSB)�̏ꍇ */
				if (MIDIEvent_IsControlChange (pTempEvent)) {
					if (MIDIEvent_GetNumber (pTempEvent) == 0) {
						long lChannel = lTempChannel;
						if (lTempChannel == -1) {
							lChannel = MIDIEvent_GetChannel (pTempEvent);
						}
						if (MIDIEvent_GetValue (pTempEvent) == 120) {
							lOutputChannelDrum[lTempPort][lChannel] = 1;
						}
						else {
							lOutputChannelDrum[lTempPort][lChannel] = 0;
						}
					}
				}
			}
			break;
		/* XG�̏ꍇ */
		case MIDITRACK_MODEXG:
			forEachEvent (pTempTrack, pTempEvent) {
				/* CC#0(Bank Select MSB)�̏ꍇ */
				if (MIDIEvent_IsControlChange (pTempEvent)) {
					if (MIDIEvent_GetNumber (pTempEvent) == 0) {
						long lChannel = lTempChannel;
						if (lTempChannel == -1) {
							lChannel = MIDIEvent_GetChannel (pTempEvent);
						}
						if (MIDIEvent_GetValue (pTempEvent) == 127) {
							lOutputChannelDrum[lTempPort][lChannel] = 1;
						}
						else {
							lOutputChannelDrum[lTempPort][lChannel] = 0;
						}
					}
				}
			}
			break;
		/* GS/88�̏ꍇ */
		case MIDITRACK_MODEGS:
		case MIDITRACK_MODE88:
			lOutputChannelDrum[lTempPort][9] = 1;
			forEachEvent (pTempTrack, pTempEvent) {
				/* �V�X�e���G�N�X�N���[�V�u�̏ꍇ */
				if (MIDIEvent_IsSysExEvent (pTempEvent)) {
					long lLen = MIDIEvent_GetLen (pTempEvent);
					unsigned char bySysxData[256];
					memset (bySysxData, 0, sizeof (bySysxData));
					MIDIEvent_GetData (pTempEvent, bySysxData, MIN (lLen, 255));
					/* �p�[�g���[�h���b�Z�[�W�̏ꍇ */
					/* {0xF0, 0x41, 0xid, 0x42, 0x12, 0x40, 0x1X, 0x15, 0xvv, 0xcs, 0xF7} */
					if (bySysxData[0] == 0xF0 &&
						bySysxData[1] == 0x41 &&
						bySysxData[3] == 0x42 &&
						bySysxData[4] == 0x12 &&
						bySysxData[5] == 0x40 &&
						(bySysxData[6] & 0xF0) == 0x10 &&
						bySysxData[7] == 0x15 &&
						bySysxData[9] == 128 - ((bySysxData[5] + bySysxData[6] + 
										bySysxData[7] + bySysxData[8]) % 128) &&
						bySysxData[10] == 0xF7) {
						long lChannel = 0;
						if (bySysxData[6] == 0x10) {
							lChannel = 9;
						}
						else if (0x11 <= bySysxData[6] && bySysxData[6] <= 0x19) {
							lChannel = bySysxData[6] - 0x11;
						}
						else {
							lChannel = bySysxData[6] - 0x10;
						}
						lOutputChannelDrum[lTempPort][lChannel] = bySysxData[8] > 0 ? 1 : 0;
					}
				}
			}
			break;
		/* Native/GMOff/GM�̏ꍇ */
		default:
			/* �o�̓`�����l��9�ɐݒ肳��Ă���ꍇ�̂݃h���� */
			if (lTempChannel == 9) {
				lOutputChannelDrum[lTempPort][lTempChannel] = 1;
			}
			else if (0 <= lTempChannel && lTempChannel < 16) {
				lOutputChannelDrum[lTempPort][lTempChannel] = 0;
			}
			break;
		}
	}

	/* �e�g���b�N�ɂ��āA�\�����[�h���u�ʏ�v���u�h�����v���ݒ� */
	forEachTrack (pMIDIData, pTempTrack) {
		long lOutputPort = MIDITrack_GetOutputPort (pTempTrack);
		long lOutputChannel = MIDITrack_GetOutputChannel (pTempTrack);
		if (0 <= lOutputPort && lOutputPort < MIDIDATA_MAXNUMPORT &&
			0 <= lOutputChannel && lOutputChannel < 16) {
			MIDITrack_SetViewMode (pTempTrack, lOutputChannelDrum[lOutputPort][lOutputChannel]);
		}
		else {
			MIDITrack_SetViewMode (pTempTrack, 0);
		}
	}
	return 1;
}


/* �t�H�[�}�b�g�擾 */
long __stdcall MIDIData_GetFormat (MIDIData* pMIDIData) {
	assert (pMIDIData);
	return pMIDIData->m_lFormat;
}

/* �t�H�[�}�b�g�ݒ� */
/* (���ӁF���̊֐��͖��f�o�b�O�ł�) */
long __stdcall MIDIData_SetFormat (MIDIData* pMIDIData, long lFormat) {
	long i, j;
	MIDITrack* pTrack = NULL;
	long lXFVersion = 0;
	assert (pMIDIData);
	assert (0 <= lFormat && lFormat <= 2);
	lXFVersion = MIDIData_GetXFVersion (pMIDIData);

	/* 0��1����0��2�̏ꍇ�A�R���_�N�^�[�g���b�N��16��MIDI�g���b�N(�v17�g���b�N)�֕��� */
	/* (XF�f�[�^�̏ꍇ�́A�Z�b�g�A�b�v��16�g���b�N��XFIH��XFKM(�v19�g���b�N)�֕���) */
	if (pMIDIData->m_lFormat == 0 && (lFormat == 1 || lFormat == 2)) {
		long lNumAddTrack = (lXFVersion == 0 ? 17 : 19);
		MIDIEvent* pEvent;
		MIDITrack* pNewTrack[19];

		/* �ǉ��p�̐V�����g���b�N�쐬 */
		for (i = 0; i < lNumAddTrack; i++) {
			pNewTrack[i] = MIDITrack_Create ();
			if (pNewTrack[i] == NULL) {
				for (j = 0; j < i; j++) {
					MIDITrack_Delete (pNewTrack[j]);
				}
				return 0;
			}
		}

		/* �e�g���b�N���Ƃ�(�g���b�N���c���Ă������) */
		while ((pTrack = pMIDIData->m_pFirstTrack)) {
			/* ���̃g���b�N���̃C�x���g���O���A�V�����g���b�N�Ɋ���U�� */
			while ((pEvent = pTrack->m_pFirstEvent)) {
				/* MIDI�`�����l���C�x���g�̏ꍇ�A�`�����l���ɉ����ăg���b�N[1]�`[16]�� */
				if (MIDIEvent_IsMIDIEvent (pEvent)) {
					MIDITrack_RemoveEvent (pTrack, pEvent);
					MIDITrack_InsertEvent (pNewTrack[MIDIEvent_GetChannel (pEvent) + 1], pEvent);
				}
				/* �G���h�I�u�g���b�N�̏ꍇ�A������폜 */
				else if (MIDIEvent_IsEndofTrack (pEvent)) {
					MIDIEvent_Delete (pEvent);
				}
				/* ���^�C�x���g�E�V�X�e���G�N�X�N���[�V���C�x���g�̏ꍇ */
				else {
					/* XF�f�[�^�̏ꍇ */
					if (lXFVersion != 0) {
						/* XF�Ŏn�܂�e�L�X�g�̓g���b�N[17]�ցA���̑��̃e�L�X�g�̓g���b�N[0]�� */
						if (pEvent->m_lKind == MIDIEVENT_TEXTEVENT) {
							if (pEvent->m_lLen >= 4) {
								if (strncmp ((char*)(pEvent->m_pData), "XF", 2) == 0) {
									MIDITrack_RemoveEvent (pTrack, pEvent);
									MIDITrack_InsertEvent (pNewTrack[17], pEvent);
								}
								else {
									MIDITrack_RemoveEvent (pTrack, pEvent);
									MIDITrack_InsertEvent (pNewTrack[0], pEvent);
								}
							}
							else {
								MIDITrack_RemoveEvent (pTrack, pEvent);
								MIDITrack_InsertEvent (pNewTrack[0], pEvent);
							}
						}
						/* �̎��̓g���b�N[18]�� */
						else if (pEvent->m_lKind == MIDIEVENT_LYRIC) {
							MIDITrack_RemoveEvent (pTrack, pEvent);
							MIDITrack_InsertEvent (pNewTrack[18], pEvent);
						}
						/* �L���[�|�C���g�̓g���b�N[18]�� */
						else if (pEvent->m_lKind == MIDIEVENT_CUEPOINT) {
							MIDITrack_RemoveEvent (pTrack, pEvent);
							MIDITrack_InsertEvent (pNewTrack[18], pEvent);
						}
						/* ���̑��̏ꍇ�A�g���b�N[0]�� */
						else {
							MIDITrack_RemoveEvent (pTrack, pEvent);
							MIDITrack_InsertEvent (pNewTrack[0], pEvent);
						}
					}
					/* �ʏ��MIDI�f�[�^�̏ꍇ�A�g���b�N[0]�� */
					else {
						MIDITrack_RemoveEvent (pTrack, pEvent);
						MIDITrack_InsertEvent (pNewTrack[0], pEvent);
					}
				}
			}
			/* ����MIDI�g���b�N���폜 */
			MIDITrack_Delete (pTrack);
		}

		/* �ǉ��p�̐V�����g���b�N��EOT������MIDIData�ɒǉ� */
		for (i = 0; i < lNumAddTrack; i++) {
			long lLastTime = 0;
			pEvent = pNewTrack[i]->m_pLastEvent;
			if (pEvent) {
				lLastTime = pEvent->m_lTime;
			}
			MIDITrack_InsertEndofTrack (pNewTrack[i], lLastTime);
			MIDIData_AddTrackForce (pMIDIData, pNewTrack[i]);
		}
	}

	/* 1��0����2��0�̏ꍇ�A�S�g���b�N��1�g���b�N�ɏW�� */
	/* (XF�̏ꍇ�AMThd�g���b�N�AXFIH�g���b�N�AXFKM�g���b�N��3�g���b�N�ɏW��) */
	else if ((pMIDIData->m_lFormat == 1 || pMIDIData->m_lFormat == 2) && lFormat == 0) {
		long lNumAddTrack = (lXFVersion == 0 ? 1 : 3);
		MIDIEvent* pEvent = NULL;
		MIDITrack* pMIDITrack = NULL;
		MIDITrack* pNewTrack[3];
		/* �V�����ǉ��p�g���b�N�m�� */
		for (i = 0; i < lNumAddTrack; i++) {
			pNewTrack[i] = MIDITrack_Create ();
			if (pNewTrack[i] == NULL) {
				for (j = 0; j < i; j++) {
					MIDITrack_Delete (pNewTrack[j]);
				}
				return 0;
			}
		}

		/* �e�g���b�N���Ƃ�(�g���b�N���c���Ă������) */
		while ((pMIDITrack = pMIDIData->m_pFirstTrack)) {
			/* ���̃g���b�N���̃C�x���g���O���A�V�����g���b�N�Ɋ���U�� */
			while ((pEvent = pMIDITrack->m_pFirstEvent)) {
				/* �g���b�N���̏ꍇ */
				if (pEvent->m_lKind == MIDIEVENT_TRACKNAME) {
					MIDITrack_RemoveEvent (pMIDITrack, pEvent);
					/* pEvent->m_lKind = MIDIEVENT_TEXTEVENT; 20090112�p�~ */
					MIDITrack_InsertEvent (pNewTrack[0], pEvent);
				}
				/* �G���h�I�u�g���b�N�̏ꍇ�A���̃C�x���g���폜 */
				else if (pEvent->m_lKind == MIDIEVENT_ENDOFTRACK) {
					MIDIEvent_Delete (pEvent);
				}
				/* ���̑��̃C�x���g�̏ꍇ */
				else {
					/* XF�f�[�^�̏ꍇ */
					if (lXFVersion != 0) {
						/* XF�Ŏn�܂�e�L�X�g�C�x���g�̓g���b�N[1]�ցA���̑��̃e�L�X�g�̓g���b�N[0]�� */
						if (pEvent->m_lKind == MIDIEVENT_TEXTEVENT) {
							if (pEvent->m_lLen >= 4) {
								if (strncmp ((char*)(pEvent->m_pData), "XF", 2) == 0) {
									MIDITrack_RemoveEvent (pMIDITrack, pEvent);
									MIDITrack_InsertEvent (pNewTrack[1], pEvent);
								}
								else {
									MIDITrack_RemoveEvent (pMIDITrack, pEvent);
									MIDITrack_InsertEvent (pNewTrack[0], pEvent);
								}
							}
							else {
								MIDITrack_RemoveEvent (pMIDITrack, pEvent);
								MIDITrack_InsertEvent (pNewTrack[0], pEvent);
							}
						}
						/* �L���[�|�C���g�̓g���b�N[2]�� */
						else if (pEvent->m_lKind == MIDIEVENT_CUEPOINT) {
							MIDITrack_RemoveEvent (pMIDITrack, pEvent);
							MIDITrack_InsertEvent (pNewTrack[2], pEvent);
						}
						/* �̎��̓g���b�N[2]�� */
						else if (pEvent->m_lKind == MIDIEVENT_LYRIC) {
							MIDITrack_RemoveEvent (pMIDITrack, pEvent);
							MIDITrack_InsertEvent (pNewTrack[2], pEvent);
						}
						/* ���̑��̓g���b�N[0]�� */
						else {
							MIDITrack_RemoveEvent (pMIDITrack, pEvent);
							MIDITrack_InsertEvent (pNewTrack[0], pEvent);
						}
					}
					/* �ʏ��MIDI�f�[�^�̏ꍇ�A���ׂăg���b�N[0]�� */
					else {
						MIDITrack_RemoveEvent (pMIDITrack, pEvent);
						MIDITrack_InsertEvent (pNewTrack[0], pEvent);
					}
				}
			}
			/* ����MIDI�g���b�N���폜 */
			MIDITrack_Delete (pMIDITrack);
		}

		/* �ǉ��p�̐V�����g���b�N��EOT������MIDIData�ɒǉ� */
		for (i = 0; i < lNumAddTrack; i++) {
			long lLastTime = 0;
			pEvent = pNewTrack[i]->m_pLastEvent;
			if (pEvent) {
				lLastTime = pEvent->m_lTime;
			}
			MIDITrack_InsertEndofTrack (pNewTrack[i], lLastTime);
			MIDIData_AddTrackForce (pMIDIData, pNewTrack[i]);
		}
	}
	/* ���̑��̃t�H�[�}�b�g�ϊ����͕ϊ��Ȃ� */
	else {
		return 0;
	}

	/* �t�H�[�}�b�g�̊m�� */
	pMIDIData->m_lFormat = lFormat;

	/* �e�g���b�N�̏o�̓|�[�g�ԍ��E�o�̓`�����l���E�\�����[�h�̎����ݒ� */
	MIDIData_UpdateOutputPort (pMIDIData);
	MIDIData_UpdateOutputChannel (pMIDIData);
	MIDIData_UpdateViewMode (pMIDIData);

	/* �e�g���b�N�̓��̓|�[�g�ԍ��E���̓`�����l���̐ݒ� */
	forEachTrack (pMIDIData, pTrack) {
		pTrack->m_lInputOn = 1;
		pTrack->m_lInputPort = pTrack->m_lOutputPort;
		pTrack->m_lInputChannel = pTrack->m_lOutputChannel;
		pTrack->m_lOutputOn = 1;
	}

	return 1;
}

/* �^�C���x�[�X�擾 */
long __stdcall MIDIData_GetTimeBase (MIDIData* pMIDIData, long* pMode, long* pResolution) {
	assert (pMIDIData);
	if (pMIDIData->m_lTimeBase & 0x00008000) {
		*pMode = 256 - ((pMIDIData->m_lTimeBase & 0x0000FF00) >> 8);
		*pResolution = pMIDIData->m_lTimeBase & 0x000000FF;
	}
	else {
		*pMode = MIDIDATA_TPQNBASE;
		*pResolution = pMIDIData->m_lTimeBase & 0x00007FFF;
	}
	return 1;
}

/* �^�C���x�[�X�̂����A�^�C�����[�h�擾 */
long __stdcall MIDIData_GetTimeMode (MIDIData* pMIDIData) {
	assert (pMIDIData);
	if (pMIDIData->m_lTimeBase & 0x00008000) {
		return 256 - ((pMIDIData->m_lTimeBase & 0x0000FF00) >> 8);
	}
	else {
		return MIDIDATA_TPQNBASE;
	}
}

/* �^�C���x�[�X�̂����A���]�����[�V�����擾 */
long __stdcall MIDIData_GetTimeResolution (MIDIData* pMIDIData) {
	assert (pMIDIData);
	if (pMIDIData->m_lTimeBase & 0x00008000) {
		return pMIDIData->m_lTimeBase & 0x000000FF;
	}
	else {
		return pMIDIData->m_lTimeBase & 0x00007FFF;
	}
}

/* �^�C���x�[�X�ݒ� */
long __stdcall MIDIData_SetTimeBase (MIDIData* pMIDIData, long lMode, long lResolution) {
	MIDIEvent* pEvent;
	MIDITrack* pTrack;
	long lOldMode;
	long lOldResolution;
	assert (pMIDIData);
	/* ����(���[�h�ƕ���\)�̐������`�F�b�N */
	if (lMode == MIDIDATA_TPQNBASE) {
		if (lResolution < MIDIDATA_MINTPQNRESOLUTION || lResolution > MIDIDATA_MAXTPQNRESOLUTION) {
			return 0;
		}
	}
	else if (lMode == MIDIDATA_SMPTE24BASE || lMode == MIDIDATA_SMPTE25BASE || 
		lMode == MIDIDATA_SMPTE29BASE || lMode == MIDIDATA_SMPTE30BASE) {
		if (lResolution < MIDIDATA_MINSMPTERESOLUTION || lResolution > MIDIDATA_MAXSMPTERESOLUTION) {
			return 0;
		}
	}
	else {
		return 0;
	}
	/* �ύX�O�̃��[�h�ƕ���\�𓾂� */
	MIDIData_GetTimeBase (pMIDIData, &lOldMode, &lOldResolution);

	/* TPQN�x�[�X����TPQN�x�[�X�֕ϊ��� */
	if (lOldMode == 0 && lMode == 0) {
		forEachTrack (pMIDIData, pTrack) {
			forEachEvent (pTrack, pEvent) {
				pEvent->m_lTime = pEvent->m_lTime * lResolution / pMIDIData->m_lTimeBase;
			}
		}
		pMIDIData->m_lTimeBase = lResolution;
	}

	/* SMPTE�x�[�X����SMPTE�x�[�X�֕ϊ��� */
	else if (lOldMode > 0 && lMode > 0) {
		long lOldModeResolution = lOldMode * lOldResolution;
		long lModeResolution = lMode * lResolution;
		forEachTrack (pMIDIData, pTrack) {
			forEachEvent (pTrack, pEvent) {
				pEvent->m_lTime = pEvent->m_lTime * lModeResolution / lOldModeResolution;
			}
		}
		pMIDIData->m_lTimeBase = ((256 - lMode) << 8) | lResolution;
	}

	/* TPQN�x�[�X����SMPTE�x�[�X�֕ϊ��� */
	else if (lOldMode == 0 && lMode > 0) {
		long lModeResolution = lMode * lResolution;
		MIDITrack* pSetupTrack = pMIDIData->m_pFirstTrack;
		/* for���[�v���Ń^�C����ǂ��̂ŁA�e���|�}�b�p�[�͍Ō�Ƀ^�C���ϊ����邱�ƁB */
		forEachTrackInverse (pMIDIData, pTrack) {
			if (pMIDIData->m_lFormat == 2) {
				pSetupTrack = pTrack;
			}
			forEachEventInverse (pTrack, pEvent) {
				pEvent->m_lTime = MIDITrack_TPQNtoSMPTE 
					(pSetupTrack, pEvent->m_lTime, lMode, lResolution);
			}
		}
		pMIDIData->m_lTimeBase = ((256 - lMode) << 8) | lResolution;
	}

	/* SMPTE�x�[�X����TPQN�x�[�X�֕ϊ��� */
	else if (lOldMode > 0 && lMode == 0) {
		long lOldModeResolution = lOldMode * lOldResolution;
		MIDITrack* pSetupTrack = pMIDIData->m_pFirstTrack;
		/* for���[�v���Ń^�C����ǂ��̂ŁA�e���|�}�b�p�[�͍Ō�Ƀ^�C���ϊ����邱�ƁB */
		forEachTrackInverse (pMIDIData, pTrack) {
			if (pMIDIData->m_lFormat == 2) {
				pSetupTrack = pTrack;
			}
			forEachEventInverse (pTrack, pEvent) {
				pEvent->m_lTime = MIDITrack_SMPTEtoTPQN (pSetupTrack, pEvent->m_lTime, lResolution); 
			}
		}
		pMIDIData->m_lTimeBase = lResolution;
	}
	return 1;
}

/* �g���b�N���擾 */
long __stdcall MIDIData_GetNumTrack (MIDIData* pMIDIData) {
	assert (pMIDIData);
	return pMIDIData->m_lNumTrack;
}

/* �g���b�N�����J�E���g���A�e�g���b�N�̃C���f�b�N�X�Ƒ��g���b�N�����X�V���A�g���b�N����Ԃ��B */
long __stdcall MIDIData_CountTrack (MIDIData* pMIDIData) {
	MIDITrack* pTrack;
	long i = 0;
	assert (pMIDIData);
	forEachTrack (pMIDIData, pTrack) {
		pTrack->m_lTempIndex = i;
		i++;
	}
	pMIDIData->m_lNumTrack = i;
	return i;
}

/* XF�ł���Ƃ��AXF�̃��@�[�W�������擾(XF�łȂ����0) */
long __stdcall MIDIData_GetXFVersion (MIDIData* pMIDIData) {
	/* �V�[�P���T�ŗL�̃C�x���g�� */
	/* {43 7B 00 58 46 Mj Mn S1 S0} ��T���A*/
	/* Mj | (Mn << 8) | (S1 << 16) | (S0 << 24) ��Ԃ��B*/
	if (pMIDIData->m_pFirstTrack) {
		return MIDITrack_GetXFVersion (pMIDIData->m_pFirstTrack);
	}
	return 0;
}

/* �ŏ��̃g���b�N�ւ̃|�C���^�擾 */
MIDITrack* __stdcall MIDIData_GetFirstTrack (MIDIData* pMIDIData) {
	assert (pMIDIData);
	return pMIDIData->m_pFirstTrack;
}

/* �Ō�̃g���b�N�ւ̃|�C���^�擾 */
MIDITrack* __stdcall MIDIData_GetLastTrack (MIDIData* pMIDIData) {
	assert (pMIDIData);
	return pMIDIData->m_pLastTrack;
}

/* �w��C���f�b�N�X��MIDI�g���b�N�ւ̃|�C���^���擾����(�Ȃ����NULL) */
MIDITrack* __stdcall MIDIData_GetTrack (MIDIData* pMIDIData, long lTrackIndex) {
	int i = 0;
	MIDITrack* pMIDITrack = NULL;
	assert (pMIDIData);
	forEachTrack (pMIDIData, pMIDITrack) {
		if (i == lTrackIndex) {
			return pMIDITrack;
		}
		i++;
	}
	return NULL;
}

/* MIDI�f�[�^�̊J�n����[Tick]��Ԃ��B */
long __stdcall MIDIData_GetBeginTime (MIDIData* pMIDIData) {
	MIDITrack* pTrack;
	MIDIEvent* pEvent;
	long lTime, lBeginTime;
	assert (pMIDIData);
	lBeginTime = LONG_MAX;
	forEachTrack (pMIDIData, pTrack) {
		pEvent = pTrack->m_pFirstEvent;
		if (pEvent) {
			lTime = pEvent->m_lTime;
			if (lBeginTime > lTime) {
				lBeginTime = lTime;	
			}
		}
	}
	return lBeginTime;
}

/* MIDI�f�[�^�̏I������[Tick]��Ԃ��B */
long __stdcall MIDIData_GetEndTime (MIDIData* pMIDIData) {
	MIDITrack* pTrack;
	MIDIEvent* pEvent;
	long lTime, lEndTime;
	assert (pMIDIData);
	lEndTime = LONG_MIN;
	forEachTrack (pMIDIData, pTrack) {
		pEvent = pTrack->m_pLastEvent;
		if (pEvent) {
			lTime = pEvent->m_lTime;
			if (lEndTime < lTime) {
				lEndTime = lTime;	
			}
		}

	}
	return lEndTime;
}

/* MIDI�f�[�^�̃^�C�g���ȈՎ擾 */
char* __stdcall MIDIData_GetTitle (MIDIData* pMIDIData, char* pData, long lLen) {
	MIDITrack* pTrack;
	MIDIEvent* pEvent;
	assert (pMIDIData);
	memset (pData, 0, lLen);
	pTrack = MIDIData_GetFirstTrack (pMIDIData);
	if (pTrack == NULL) {
		return pData;
	}
	forEachEvent (pTrack, pEvent) {
		if (MIDIEvent_IsTrackName (pEvent)) {
			break;
		}
	}
	if (pEvent == NULL) {
		return pData;
	}
	return MIDIEvent_GetText (pEvent, pData, lLen);

}

/* MIDI�f�[�^�̃^�C�g���ȈՐݒ� */
long __stdcall MIDIData_SetTitle (MIDIData* pMIDIData, const char* pszText) {
	MIDITrack* pTrack;
	MIDIEvent* pEvent;
	assert (pMIDIData);
	pTrack = MIDIData_GetFirstTrack (pMIDIData);
	if (pTrack == NULL) {
		return 0;
	}
	forEachEvent (pTrack, pEvent) {
		if (MIDIEvent_IsTrackName (pEvent)) {
			break;
		}
	}
	if (pEvent == NULL && pszText != NULL) {
		return MIDITrack_InsertTrackName (pTrack, 0, pszText);
	}
	else if (pEvent == NULL && pszText == NULL) {
		return 1;
	}
	else if (pEvent != NULL && pszText == NULL) {
		MIDIEvent_DeleteSingle (pEvent);
		return 1;
	}
	return MIDIEvent_SetText (pEvent, pszText);
}

/* MIDI�f�[�^�̃T�u�^�C�g���ȈՎ擾 */
char* __stdcall MIDIData_GetSubTitle (MIDIData* pMIDIData, char* pData, long lLen) {
	MIDITrack* pTrack;
	MIDIEvent* pEvent;
	long nCount = 0;
	assert (pMIDIData);
	memset (pData, 0, lLen);
	pTrack = MIDIData_GetFirstTrack (pMIDIData);
	if (pTrack == NULL) {
		return pData;
	}
	forEachEvent (pTrack, pEvent) {
		if (MIDIEvent_IsTrackName (pEvent)) {			
			if (nCount >= 1) {
				break;
			}
			nCount++;
		}
	}
	if (pEvent == NULL) {
		return pData;
	}
	return MIDIEvent_GetText (pEvent, pData, lLen);

}

/* MIDI�f�[�^�̃T�u�^�C�g���ȈՐݒ� */
long __stdcall MIDIData_SetSubTitle (MIDIData* pMIDIData, const char* pszText) {
	MIDITrack* pTrack;
	MIDIEvent* pEvent;
	long nCount = 0;
	assert (pMIDIData);
	pTrack = MIDIData_GetFirstTrack (pMIDIData);
	if (pTrack == NULL) {
		return 0;
	}
	forEachEvent (pTrack, pEvent) {
		if (MIDIEvent_IsTrackName (pEvent)) {			
			if (nCount >= 1) {
				break;
			}
			nCount++;
		}
	}
	if (pEvent == NULL && pszText != NULL) {
		return MIDITrack_InsertTrackName (pTrack, 0, pszText);
	}
	else if (pEvent == NULL && pszText == NULL) {
		return 1;
	}
	else if (pEvent != NULL && pszText == NULL) {
		MIDIEvent_DeleteSingle (pEvent);
		return 1;
	}
	return MIDIEvent_SetText (pEvent, pszText);
}

/* MIDI�f�[�^�̒��쌠�ȈՎ擾 */
char* __stdcall MIDIData_GetCopyright (MIDIData* pMIDIData, char* pData, long lLen) {
	MIDITrack* pTrack;
	MIDIEvent* pEvent;
	assert (pMIDIData);
	memset (pData, 0, lLen);
	pTrack = MIDIData_GetFirstTrack (pMIDIData);
	if (pTrack == NULL) {
		return pData;
	}
	forEachEvent (pTrack, pEvent) {
		if (MIDIEvent_IsCopyrightNotice (pEvent)) {
			break;
		}
	}
	if (pEvent == NULL) {
		return pData;
	}
	return MIDIEvent_GetText (pEvent, pData, lLen);

}

/* MIDI�f�[�^�̒��쌠�ȈՐݒ� */
long __stdcall MIDIData_SetCopyright (MIDIData* pMIDIData, const char* pszText) {
	MIDITrack* pTrack;
	MIDIEvent* pEvent;
	assert (pMIDIData);
	pTrack = MIDIData_GetFirstTrack (pMIDIData);
	if (pTrack == NULL) {
		return 0;
	}
	forEachEvent (pTrack, pEvent) {
		if (MIDIEvent_IsCopyrightNotice (pEvent)) {
			break;
		}
	}
	if (pEvent == NULL && pszText != NULL) {
		return MIDITrack_InsertCopyrightNotice (pTrack, 0, pszText);
	}
	else if (pEvent == NULL && pszText == NULL) {
		return 1;
	}
	else if (pEvent != NULL && pszText == NULL) {
		MIDIEvent_DeleteSingle (pEvent);
		return 1;
	}
	else {
		return MIDIEvent_SetText (pEvent, pszText);
	}
}

/* MIDI�f�[�^�̃R�����g�ȈՎ擾 */
char* __stdcall MIDIData_GetComment (MIDIData* pMIDIData, char* pData, long lLen) {
	MIDITrack* pTrack;
	MIDIEvent* pEvent;
	assert (pMIDIData);
	memset (pData, 0, lLen);
	pTrack = MIDIData_GetFirstTrack (pMIDIData);
	if (pTrack == NULL) {
		return 0;
	}
	forEachEvent (pTrack, pEvent) {
		if (MIDIEvent_IsTextEvent (pEvent)) {
			break;
		}
	}
	if (pEvent == NULL) {
		return 0;
	}
	return MIDIEvent_GetText (pEvent, pData, lLen);

}

/* MIDI�f�[�^�̃R�����g�ȈՐݒ� */
long __stdcall MIDIData_SetComment (MIDIData* pMIDIData, const char* pszText) {
	MIDITrack* pTrack;
	MIDIEvent* pEvent;
	assert (pMIDIData);
	pTrack = MIDIData_GetFirstTrack (pMIDIData);
	if (pTrack == NULL) {
		return 0;
	}
	forEachEvent (pTrack, pEvent) {
		if (MIDIEvent_IsTextEvent (pEvent)) {
			break;
		}
	}
	if (pEvent == NULL && pszText != NULL) {
		return MIDITrack_InsertTextEvent (pTrack, 0, pszText);
	}
	else if (pEvent == NULL && pszText == NULL) {
		return 1;
	}
	else if (pEvent != NULL && pszText == NULL) {
		MIDIEvent_DeleteSingle (pEvent);
		return 1;
	}
	return MIDIEvent_SetText (pEvent, pszText);
}

/* �^�C���X�^���v���~���b�����ɕϊ� */
long __stdcall MIDIData_TimeToMillisec (MIDIData* pMIDIData, long lTime) {
	long lTimeMode, lTimeResolution;
	MIDIData_GetTimeBase (pMIDIData, &lTimeMode, &lTimeResolution);
	if (lTimeMode == MIDIDATA_TPQNBASE) {
		if (pMIDIData->m_pFirstTrack) {
			return MIDITrack_TimeToMillisec (pMIDIData->m_pFirstTrack, lTime);
		}
		return 0;
	}
	else {
		switch (lTimeMode) {
		case MIDIDATA_SMPTE24BASE:
			return lTime * 1000 / lTimeResolution / 24;
		case MIDIDATA_SMPTE25BASE:
			return lTime * 1000 / lTimeResolution / 25;
		case MIDIDATA_SMPTE29BASE:
			return lTime * 1000 / lTimeResolution * 1000 / 2997;
		case MIDIDATA_SMPTE30BASE:
			return lTime * 1000 / lTimeResolution / 30;
		default:
			return 0;
		}
	}
}

/* �~���b�������^�C���X�^���v�ɕϊ� */
long __stdcall MIDIData_MillisecToTime (MIDIData* pMIDIData, long lMillisec) {
	long lTimeMode, lTimeResolution;
	MIDIData_GetTimeBase (pMIDIData, &lTimeMode, &lTimeResolution);
	if (lTimeMode == MIDIDATA_TPQNBASE) {
		if (pMIDIData->m_pFirstTrack) {
			return MIDITrack_MillisecToTime (pMIDIData->m_pFirstTrack, lMillisec);
		}
		return 0;
	}
	else {
		switch (lTimeMode) {
		case MIDIDATA_SMPTE24BASE:
			return lTimeResolution * lMillisec * 24 / 1000;
		case MIDIDATA_SMPTE25BASE:
			return lTimeResolution * lMillisec * 25 / 1000;
		case MIDIDATA_SMPTE29BASE:
			return lTimeResolution * lMillisec * 2997 / 1000 / 1000;
		case MIDIDATA_SMPTE30BASE:
			return lTimeResolution * lMillisec * 30 / 1000;
		default:
			return 0;
		}
	}
}

/* �^�C���R�[�h�����߁F���F�e�B�b�N�ɕ���(�ŏ��̃g���b�N���̔��q�L������v�Z) */
long __stdcall MIDIData_BreakTimeEx
(MIDIData* pMIDIData, long lTime, long* pMeasure, long* pBeat, long* pTick,
 long* pnn, long* pdd, long* pcc, long* pbb) {
	if (pMIDIData->m_pFirstTrack) {
		return MIDITrack_BreakTimeEx (pMIDIData->m_pFirstTrack,
			lTime, pMeasure, pBeat, pTick, pnn, pdd, pcc, pbb);
	}
	return 0;
}

/* �^�C���R�[�h�����߁F���F�e�B�b�N�ɕ���(�ŏ��̃g���b�N���̔��q�L������v�Z) */
long __stdcall MIDIData_BreakTime 
(MIDIData* pMIDIData, long lTime, long* pMeasure, long* pBeat, long* pTick) {
	if (pMIDIData->m_pFirstTrack) {
		return MIDITrack_BreakTime (pMIDIData->m_pFirstTrack, 
			lTime, pMeasure, pBeat, pTick);
	}
	return 0;
}

/* ���߁F���F�e�B�b�N����^�C���R�[�h�𐶐�(�ŏ��̃g���b�N���̔��q�L������v�Z) */
long __stdcall MIDIData_MakeTimeEx 
(MIDIData* pMIDIData, long lMeasure, long lBeat, long lTick, 
 long* pTime, long* pnn, long* pdd, long* pcc, long* pbb) {
	if (pMIDIData->m_pFirstTrack) {
		return MIDITrack_MakeTimeEx (pMIDIData->m_pFirstTrack,
			lMeasure, lBeat, lTick, pTime, pnn, pdd, pcc, pbb);
	}
	return 0;
}

/* ���߁F���F�e�B�b�N����^�C���R�[�h�𐶐�(�ŏ��̃g���b�N���̔��q�L������v�Z) */
long __stdcall MIDIData_MakeTime 
(MIDIData* pMIDIData, long lMeasure, long lBeat, long lTick, long* pTime) {
	if (pMIDIData->m_pFirstTrack) {
		return MIDITrack_MakeTime (pMIDIData->m_pFirstTrack, 
			lMeasure, lBeat, lTick, pTime);
	}
	return 0;
}

/* �w��ʒu�̒��O�̃e���|���擾 */
long __stdcall MIDIData_FindTempo
(MIDIData* pMIDIData, long lTime, long* pTempo) {
	if (pMIDIData->m_pFirstTrack) {
		return MIDITrack_FindTempo (pMIDIData->m_pFirstTrack, lTime, pTempo);
	}
	return 0;
}

/* �w��ʒu�̒��O�̔��q�L�����擾 */
long __stdcall MIDIData_FindTimeSignature
(MIDIData* pMIDIData, long lTime, long* pnn, long* pdd, long* pcc, long* pbb) {
	if (pMIDIData->m_pFirstTrack) {
		return MIDITrack_FindTimeSignature
			(pMIDIData->m_pFirstTrack, lTime, pnn, pdd, pcc, pbb);
	}
	return 0;
}

/* �w��ʒu�̒��O�̒����L�����擾 */
long __stdcall MIDIData_FindKeySignature
(MIDIData* pMIDIData, long lTime, long* psf, long* pmi) {
	if (pMIDIData->m_pFirstTrack) {
		return MIDITrack_FindKeySignature 
			(pMIDIData->m_pFirstTrack, lTime, psf, pmi);
	}
	return 0;
}

/* MIDI�f�[�^�ɕʂ�MIDI�f�[�^���}�[�W���� */
long __stdcall MIDIData_Merge (MIDIData* pMIDIData, MIDIData* pMergeData, 
	long lTime, long lFlags, long* pInsertedEventCount, long* pDeletedEventCount) {
	
	long lNumMergeTrack = MIDIData_CountTrack (pMergeData);
	long lTimeMode, lTimeResolution;
	long ret;
	MIDITrack* pMergeTrack;
	MIDIEvent* pMergeEvent;
	MIDITrack* pMIDITrack = pMIDIData->m_pFirstTrack;
	MIDIEvent* pMIDIEvent;
	*pInsertedEventCount = 0;
	*pDeletedEventCount = 0;
	
	/* �}�[�W�pMIDI�f�[�^�̃t�H�[�}�b�g������MIDI�f�[�^�̃t�H�[�}�b�g�ɍ��킹��B */
	if (pMIDIData->m_lFormat != pMergeData->m_lFormat) {
		MIDIData_SetFormat (pMergeData, pMIDIData->m_lFormat); 
	}

	/* �}�[�W�pMIDI�f�[�^�̃^�C���x�[�X������MIDI�f�[�^�̃^�C���x�[�X�ɍ��킹��B */
	MIDIData_GetTimeBase (pMIDIData, &lTimeMode, &lTimeResolution);
	if (pMIDIData->m_lTimeBase != pMergeData->m_lTimeBase) {
		MIDIData_SetTimeBase (pMergeData, lTimeMode, lTimeResolution);
	}

	/* �}����g���b�N�̑��݌��� */
	pMIDITrack = pMIDIData->m_pFirstTrack;
	forEachTrack (pMergeData, pMergeTrack) {
		/* �g���b�N������Ȃ��ꍇ�͒ǉ����� */
		if (pMIDITrack == NULL) {
			MIDITrack* pNewTrack = MIDITrack_Create ();
			MIDIData_AddTrack (pMIDIData, pNewTrack);
			pMIDITrack = pNewTrack;
		}
	}

	/* �d���C�x���g���폜����t���O�������Ă���Ȃ�� */
	if (lFlags == 1) {
		pMIDITrack = pMIDIData->m_pFirstTrack;
		forEachTrack (pMergeData, pMergeTrack) {
			forEachEvent (pMergeTrack, pMergeEvent) {
				/* ����MIDI�f�[�^�ɏd���C�x���g���Ȃ����`�F�b�N���A����΍폜 */			
				pMIDIEvent = pMIDITrack->m_pFirstEvent;
				while (pMIDIEvent) {
					if (MIDIEvent_GetTime (pMergeEvent) + lTime == 
						MIDIEvent_GetTime (pMIDIEvent)) {
						if (MIDIEvent_GetKind (pMergeEvent) ==
							MIDIEvent_GetKind (pMIDIEvent)) {
							if ((MIDIEvent_IsNoteOff (pMergeEvent) &&
								MIDIEvent_IsNoteOff (pMIDIEvent)) ||
								(MIDIEvent_IsNoteOn (pMergeEvent) &&
								MIDIEvent_IsNoteOn (pMIDIEvent)) ||
								(MIDIEvent_IsKeyAftertouch (pMergeEvent) &&
								MIDIEvent_IsKeyAftertouch (pMIDIEvent))) {
								if (MIDIEvent_GetChannel (pMergeEvent) == 
									MIDIEvent_GetChannel (pMIDIEvent) &&
									MIDIEvent_GetKey (pMergeEvent) == 
									MIDIEvent_GetKey (pMIDIEvent)) {
									ret = MIDIEvent_Delete (pMIDIEvent);
									*pDeletedEventCount += ret;
									pMIDIEvent = pMIDITrack->m_pFirstEvent;
									continue;
								}
							}
							else if (MIDIEvent_IsControlChange (pMergeEvent) &&
								MIDIEvent_IsControlChange (pMIDIEvent)) {
								if (MIDIEvent_GetChannel (pMergeEvent) == 
									MIDIEvent_GetChannel (pMIDIEvent) &&
									MIDIEvent_GetNumber (pMergeEvent) == 
									MIDIEvent_GetNumber (pMIDIEvent)) {
									ret = MIDIEvent_Delete (pMIDIEvent);
									*pDeletedEventCount += ret;
									pMIDIEvent = pMIDITrack->m_pFirstEvent;
									continue;
								}
							}
							else if ((MIDIEvent_IsProgramChange (pMergeEvent) &&
								MIDIEvent_IsProgramChange (pMIDIEvent)) ||
								(MIDIEvent_IsChannelAftertouch (pMergeEvent) &&
								MIDIEvent_IsChannelAftertouch (pMIDIEvent)) ||
								(MIDIEvent_IsPitchBend (pMergeEvent) &&
								MIDIEvent_IsPitchBend (pMIDIEvent))) {
								if (MIDIEvent_GetChannel (pMergeEvent) ==
									MIDIEvent_GetChannel (pMIDIEvent)) {
									ret = MIDIEvent_Delete (pMIDIEvent);
									*pDeletedEventCount += ret;
									pMIDIEvent = pMIDITrack->m_pFirstEvent;
									continue;
								}
							}
							else if ((MIDIEvent_IsSysExEvent (pMergeEvent) &&
								MIDIEvent_IsSysExEvent (pMIDIEvent)) ||
								(MIDIEvent_IsSequencerSpecific (pMergeEvent) &&
								MIDIEvent_IsSequencerSpecific (pMIDIEvent))) {
								if (MIDIEvent_GetLen (pMergeEvent) ==
									MIDIEvent_GetLen (pMIDIEvent) &&
									pMergeEvent->m_pData != NULL && /* 20091024�������ǉ� */
									pMIDIEvent->m_pData != NULL) { /* 20091024�������ǉ� */
									if (memcmp (pMergeEvent->m_pData,
										pMIDIEvent->m_pData, pMergeEvent->m_lLen) == 0) {
										ret = MIDIEvent_Delete (pMIDIEvent);
										*pDeletedEventCount += ret;
										pMIDIEvent = pMIDITrack->m_pFirstEvent;
										continue;
									}
								}									
							}
							else {
								ret = MIDIEvent_Delete (pMIDIEvent);
								*pDeletedEventCount += ret;
								pMIDIEvent = pMIDITrack->m_pFirstEvent;
								continue;
							}
						}
					}
					pMIDIEvent = pMIDIEvent->m_pNextEvent;
				}
			}
			pMIDITrack = pMIDITrack->m_pNextTrack;
		}
	}

	/* �}�[�W�pMIDI�f�[�^����C�x���g���������A����MIDI�f�[�^�ɑ}�� */
	pMIDITrack = pMIDIData->m_pFirstTrack;
	forEachTrack (pMergeData, pMergeTrack) {
		pMergeEvent = pMergeTrack->m_pFirstEvent;
		while (pMergeEvent) {
			ret = MIDITrack_RemoveEvent (pMergeTrack, pMergeEvent);
			ret = MIDIEvent_SetTime (pMergeEvent, pMergeEvent->m_lTime + lTime);
			ret = MIDITrack_InsertEvent (pMIDITrack, pMergeEvent);
			*pInsertedEventCount += ret;
			pMergeEvent = pMergeTrack->m_pFirstEvent;
		}
		pMIDITrack = pMIDITrack->m_pNextTrack;
	}
	return 1;
}


/* �ۑ��E�ǂݍ��ݗp�֐� */

/* �r�b�O�G���f�B�A�������g���G���f�B�A���ɕϊ�����(4�o�C�g) */
static unsigned long BigToLittle4 (unsigned char* pData) {
	return (*pData << 24) | (*(pData + 1) << 16) |
		(*(pData+2) << 8) | *(pData + 3);
}

/* �r�b�O�G���f�B�A�������g���G���f�B�A���ɕϊ�����(2�o�C�g) */
static unsigned short BigToLittle2 (unsigned char* pData) {
	return (unsigned short)((*pData << 8) | *(pData + 1));
}

/* ���g���G���f�B�A�����r�b�O�G���f�B�A���ɕϊ�����(4�o�C�g) */
static void LittleToBig4 (long l, unsigned char* pData) {
	*(pData + 0) = (unsigned char)((l & 0xFF000000) >> 24);
	*(pData + 1) = (unsigned char)((l & 0x00FF0000) >> 16);
	*(pData + 2) = (unsigned char)((l & 0x0000FF00) >> 8);
	*(pData + 3) = (unsigned char)(l & 0x000000FF);
}

/* ���g���G���f�B�A�����r�b�O�G���f�B�A���ɕϊ�����(2�o�C�g) */
static void LittleToBig2 (short s, unsigned char* pData) {
	*(pData + 0) = (unsigned char)((s & 0x0000FF00) >> 8);
	*(pData + 1) = (unsigned char)(s & 0x000000FF);
}


/* �ϒ������l��4�o�C�glong�^�����l�ɕϊ����A�ǂݍ��񂾃o�C�g����Ԃ��B */
static long VariableToLong (unsigned char* pData, long* pValue) {
	unsigned long ulValue = 0;
	long i = 0;
	while (1) {
		if (*(pData + i) & 0x80) { /* 7�r�b�g�ڂ������Ă��� */
			ulValue = (ulValue << 7) | (*(pData + i) & 0x7F);
			i++;
		}
		else {
			ulValue = (ulValue << 7) | (*(pData + i) & 0x7F);
			break;
		}
	}
	*pValue = (long)ulValue;
	return i + 1;
}

/* 4�o�C�glong�^�����l���ϒ������l�ɕϊ����A�������񂾃o�C�g����Ԃ�(�ő�5�o�C�g)�B */
static long LongToVariable (long lValue, unsigned char* pData) {
	unsigned long ulValue = (unsigned long)lValue;
	if (0 <= ulValue && ulValue < 128) {
		*pData = (unsigned char)ulValue;
		return 1;
	}
	else if (128 <= ulValue && ulValue < 16384) {
		*pData = (unsigned char)(((ulValue & 0x3F80) >> 7) | 0x80);
		*(pData + 1) = (unsigned char)(ulValue & 0x007F);
		return 2;
	}
	else if (16384 <= ulValue && ulValue < 2097152) { 
		*pData = (unsigned char)(((ulValue & 0x1FC000) >> 14) | 0x80);
		*(pData + 1) = (unsigned char)(((ulValue & 0x003F80) >> 7) | 0x80);
		*(pData + 2) = (unsigned char)(ulValue & 0x00007F);
		return 3;
	}
	else if (2097152 <= ulValue && ulValue < 268435456) { 
		*pData = (unsigned char)(((ulValue & 0x0FE00000) >> 21) | 0x80);
		*(pData + 1) = (unsigned char)(((ulValue & 0x001FC000) >> 14) | 0x80);
		*(pData + 2) = (unsigned char)(((ulValue & 0x00003F80) >> 7) | 0x80);
		*(pData + 3) = (unsigned char)(ulValue & 0x0000007F);
		return 4;
	}
	else if (268435456 <= ulValue) { 
		*(pData + 0) = (unsigned char)(((ulValue & 0xF0000000) >> 28) | 0x80);
		*(pData + 1) = (unsigned char)(((ulValue & 0x0FE00000) >> 21) | 0x80);
		*(pData + 2) = (unsigned char)(((ulValue & 0x001FC000) >> 14) | 0x80);
		*(pData + 3) = (unsigned char)(((ulValue & 0x00003F80) >> 7) | 0x80);
		*(pData + 4) = (unsigned char)(ulValue & 0x0000007F);
		return 5;
	}
	*pData = 0;
	return 0;
}

/* �g���b�N�ɒP��̃C�x���g�������ǉ�(�����B��)(20090712�ǉ�) */
/* pEvent�������I��pMIDITrack�̍Ō�ɑ}������B*/
/* �����`�F�b�N�A�������`�F�b�N�͍s��Ȃ��B */
long __stdcall MIDITrack_AddSingleEventForce (MIDITrack* pTrack, MIDIEvent* pInsertEvent) {
	assert (pTrack);
	assert (pInsertEvent);
	assert (pInsertEvent->m_pParent == NULL);
	/* �Ō�̃C�x���g�̒���ɑ}������ꍇ */
	if (pTrack->m_pLastEvent) {
		MIDIEvent* pLastEvent = pTrack->m_pLastEvent;
		pInsertEvent->m_pParent = pTrack;
		/* �O��̃C�x���g�̃|�C���^�̂Ȃ����� */
		pInsertEvent->m_pNextEvent = NULL;
		pInsertEvent->m_pPrevEvent = pLastEvent;
		pTrack->m_pLastEvent = pInsertEvent;
		pLastEvent->m_pNextEvent = pInsertEvent;
		/* �O��̓���C�x���g�|�C���^�ݒ� */
		pInsertEvent->m_pPrevSameKindEvent = MIDIEvent_SearchPrevSameKindEvent (pInsertEvent);
		if (pInsertEvent->m_pPrevSameKindEvent) {
			pInsertEvent->m_pPrevSameKindEvent->m_pNextSameKindEvent = pInsertEvent;
		}
		pInsertEvent->m_pNextSameKindEvent = MIDIEvent_SearchNextSameKindEvent (pInsertEvent);
		if (pInsertEvent->m_pNextSameKindEvent) {
			pInsertEvent->m_pNextSameKindEvent->m_pPrevSameKindEvent = pInsertEvent;
		}
		pTrack->m_lNumEvent ++;
	}
	/* ��g���b�N�ɑ}������ꍇ */
	else {
		pInsertEvent->m_pParent = pTrack;
		pInsertEvent->m_pNextEvent = NULL;
		pInsertEvent->m_pPrevEvent = NULL;
		pInsertEvent->m_pNextSameKindEvent = NULL;
		pInsertEvent->m_pPrevSameKindEvent = NULL;
		pTrack->m_pFirstEvent = pInsertEvent;
		pTrack->m_pLastEvent = pInsertEvent;
		pTrack->m_lNumEvent ++;
	}
	return 1;
}

/* MIDITrack��SMF�̃������u���b�N����ǂݍ���(����J) */
MIDITrack* __stdcall MIDITrack_LoadFromSMF (unsigned char* pTrackData, long lTrackLen) {

	unsigned char* p = NULL;
	long lLen = 0;
	long lDeltaTime = 0;
	long lTickCount = 0;
	unsigned char byEventKind = 0;
	unsigned char byMetaKind = 0; 
	unsigned char byOldEventKind = 0;
	MIDITrack* pTrack = NULL;
	MIDIEvent* pEvent = NULL;

	p = pTrackData;

	pTrack = MIDITrack_Create ();
	if (pTrack == NULL) {
		return NULL;
	}

	while (p < pTrackData + lTrackLen) {
		p += VariableToLong (p, &lDeltaTime);
		lTickCount += lDeltaTime;
		byEventKind = *p;
		/* ���^�C�x���g�̏ꍇ */
		if (byEventKind == 0xFF) {
			p ++; /* �C�x���g�^�C�v(0xFF)��ǂݔ�΂� */
			byMetaKind = (*p++);
			p += VariableToLong (p, &lLen);
			pEvent = MIDIEvent_Create (lTickCount, byMetaKind, p, lLen);
			p += lLen;
		}
		/* �V�X�e���G�N�X�N���[�V�u�C�x���g�̏ꍇ */
		else if (byEventKind == 0xF0 || byEventKind == 0xF7) {
			p ++; /* �C�x���g�^�C�v(0xF0||0xF7)��ǂݔ�΂� */
			p += VariableToLong (p, &lLen);
			pEvent = MIDIEvent_Create (lTickCount, byEventKind, p, lLen);
			p += lLen;
		}
		/* MIDI�C�x���g�̏ꍇ */
		else if (0x00 <= byEventKind && byEventKind <= 0xEF) {
			if (0x00 <= byEventKind && byEventKind <= 0x7F) { /* �����j���O�X�e�[�^�X */
				byEventKind = byOldEventKind;
				lLen = 2;
			}
			else {
				lLen = 3;
			}
			if (0xC0 <= byEventKind && byEventKind <= 0xDF) {
				lLen --;
			}
			pEvent = MIDIEvent_Create (lTickCount, byEventKind, p, lLen);
			p += lLen;
			byOldEventKind = byEventKind;
		}
		if (pEvent == NULL) {
			;
		}
		MIDITrack_AddSingleEventForce (pTrack, pEvent); // 20090712�ǉ�
	}

	/* �����ł���C�x���g�͌�������(�p�~) */
	/*forEachEvent (pTrack, pEvent) {
		MIDIEvent_Combine (pEvent);
	}*/

	return pTrack;
}

/* MIDIData���X�^���_�[�hMIDI�t�@�C��(SMF)����ǂݍ��݁A */
/* �V����MIDI�f�[�^�ւ̃|�C���^��Ԃ�(���s��NULL) */
MIDIData* __stdcall MIDIData_LoadFromSMF (const char* pszFileName) {
	MIDIData* pMIDIData;
	MIDITrack* pTrack;
	FILE* pFile;
	long lTrackLen = 0;
	unsigned long lFormat = 0;
	unsigned long lNumTrack = 0;
	unsigned long lTimeBase = 120;
	unsigned long i = 0;
	unsigned char* pTrackData = NULL;
	unsigned char byBuf[256];

	pMIDIData = malloc (sizeof (MIDIData));
	memset (pMIDIData, 0, sizeof (MIDIData));
	if (pMIDIData == NULL) {
		return NULL;
	}
	pMIDIData->m_pFirstTrack = NULL;
	pMIDIData->m_pLastTrack = NULL;
	pMIDIData->m_pNextSeq = NULL;
	pMIDIData->m_pPrevSeq = NULL;
	pMIDIData->m_pParent = NULL;
	pMIDIData->m_lReserved1 = 0;
	pMIDIData->m_lReserved2 = 0;
	pMIDIData->m_lReserved3 = 0;
	pMIDIData->m_lReserved4 = 0;
	pMIDIData->m_lUser1 = 0;
	pMIDIData->m_lUser2 = 0;
	pMIDIData->m_lUser3 = 0;
	pMIDIData->m_lUserFlag = 0;

	pFile = fopen (pszFileName, "rb");
	if (pFile == NULL) {
		free (pMIDIData);
		return NULL;
	}
	if (fread (byBuf, sizeof(char), 14, pFile) < 14) {
		free (pMIDIData);
		fclose (pFile);
		return NULL;
	}
	if (memcmp (byBuf, "MThd", 4) != 0) {
		free (pMIDIData);
		fclose (pFile);
		return NULL; 
	}
	lFormat = (long)BigToLittle2 (byBuf + 8);
	lNumTrack = (long)BigToLittle2 (byBuf + 10);
	lTimeBase = (long)BigToLittle2 (byBuf + 12);

	pMIDIData->m_lFormat = lFormat;
	pMIDIData->m_lNumTrack = 0;
	pMIDIData->m_lTimeBase = lTimeBase;

	while (!feof (pFile)) {

		if (fread (byBuf, sizeof(char), 8, pFile) < 8) {
			break;
		}
		if (memcmp (byBuf, "MTrk", 4) != 0 &&
			!(memcmp (byBuf, "XFIH", 4) == 0 && lFormat == 0 && i == 1) &&
			!(memcmp (byBuf, "XFKM", 4) == 0 && lFormat == 0 && i == 2)) {
			fclose (pFile); /* This is not MIDITrack. */
			return NULL;
		}
		lTrackLen = (long)BigToLittle4 (byBuf + 4);
		pTrackData = malloc (lTrackLen);
		if (pTrackData == NULL) {
			fclose (pFile); /* Out of Memory for TrackData. */
			return NULL;
		}
		if (fread (pTrackData, sizeof (char), 
			lTrackLen, pFile) < (unsigned long)lTrackLen) {
			fclose (pFile); /* There is too few TrackData. */
			free (pTrackData);
			return NULL;
		}

		pTrack = MIDITrack_LoadFromSMF (pTrackData, lTrackLen);
		if (pTrack == NULL) {
			fclose (pFile);
			return NULL;
		}
		
		MIDIData_AddTrackForce (pMIDIData, pTrack);
	
		free (pTrackData);
		pTrackData = NULL;
		i++;
	}

	pMIDIData->m_lNumTrack = i;
	assert (pMIDIData->m_lTimeBase == lTimeBase);

	/* �e�g���b�N�̏o�̓|�[�g�ԍ��E�o�̓`�����l���E�\�����[�h�̎����ݒ� */
	MIDIData_UpdateOutputPort (pMIDIData);
	MIDIData_UpdateOutputChannel (pMIDIData);
	MIDIData_UpdateViewMode (pMIDIData);

	/* �e�g���b�N�̓��̓|�[�g�ԍ��E���̓`�����l���̐ݒ� */
	forEachTrack (pMIDIData, pTrack) {
		pTrack->m_lInputOn = 1;
		pTrack->m_lInputPort = pTrack->m_lOutputPort;
		pTrack->m_lInputChannel = pTrack->m_lOutputChannel;
		pTrack->m_lOutputOn = 1;
	}
	
	fclose (pFile);
	pFile = NULL;

	return pMIDIData;

}


/* long�^�����l���ϒ������l�ɕϊ�����B*/
/* �������񂾃o�C�g����Ԃ�(�ő�4�o�C�g)�B */
static long LongToExpand (long lValue, unsigned char* pData) {
	if (0 <= lValue && lValue < 128) {
		*pData = (unsigned char)lValue;
		return 1;
	}
	else if (128 <= lValue && lValue < 16384) {
		*pData = (unsigned char)(((lValue & 0x3F80) >> 7) | 0x80);
		*(pData + 1) = (unsigned char)(lValue & 0x007F);
		return 2;
	}
	else if (16384 <= lValue && lValue < 16384 * 128) { 
		*pData = (unsigned char)(((lValue & 0x1FC000) >> 14) | 0x80);
		*(pData + 1) = (unsigned char)(((lValue & 0x003F80) >> 7) | 0x80);
		*(pData + 2) = (unsigned char)(lValue & 0x00007F);
		return 3;
	}
	else if (16384 * 128 <= lValue && lValue < 16384 * 16384) { 
		*pData = (unsigned char)(((lValue & 0x0FE00000) >> 21) | 0x80);
		*(pData + 1) = (unsigned char)(((lValue & 0x001FC000) >> 14) | 0x80);
		*(pData + 2) = (unsigned char)(((lValue & 0x00003F80) >> 7) | 0x80);
		*(pData + 3) = (unsigned char)(lValue & 0x0000007F);
		return 4;
	}
	*pData = 0;
	return 0;
}

/* MIDITrack��SMF�Ƃ��ă������u���b�N��ɕۑ������Ƃ��̒����𐄒�(����J) */
long __stdcall MIDITrack_GuessTrackDataLenAsSMF (MIDITrack* pMIDITrack) {
	long lLen = 0;
	MIDIEvent* pEvent;
	forEachEvent (pMIDITrack, pEvent) {
		lLen += 4;
		if (MIDIEvent_IsMIDIEvent (pEvent) == 0) {
			lLen += 5;	
		}
		lLen += pEvent->m_lLen;
	}
	return lLen;
}

/* MIDITrack��SMF�Ƃ��ă������u���b�N��ɕۑ�(����J) */
long __stdcall MIDITrack_SaveAsSMF (MIDITrack* pMIDITrack, unsigned char* pBuf) {
	long lLen, lDeltaTime;
	unsigned char* p = pBuf;
	MIDIEvent* pEvent;
	unsigned char cOldEventType = 0xFF;
	forEachEvent (pMIDITrack, pEvent) {
		/* �f���^�^�C���ۑ� */
		if (pEvent->m_pPrevEvent) {
			lDeltaTime = pEvent->m_lTime - pEvent->m_pPrevEvent->m_lTime;
		}
		// 20081031:�擪�󔒂͍폜���Ȃ�
		else {
			lDeltaTime = pEvent->m_lTime;
		}
		lLen = LongToExpand (lDeltaTime, p);
		p += lLen;
		/* ���^�C�x���g */
		if (0x00 <= pEvent->m_lKind && pEvent->m_lKind < 0x80) {
			*p++ = 0xFF;
			*p++ = (unsigned char)(pEvent->m_lKind);
			p += LongToExpand (pEvent->m_lLen, p);
			if (pEvent->m_pData && pEvent->m_lLen > 0) { /* 20091024�������ǉ� */
				memcpy (p, pEvent->m_pData, pEvent->m_lLen);
			}
			p += pEvent->m_lLen;
			cOldEventType = 0xFF;
		}
		/* MIDI�C�x���g */
		else if (0x80 <= pEvent->m_lKind && pEvent->m_lKind <= 0xEF) {
			/* �����j���O�X�e�[�^�X�g�p�\(2�o�C�g�ڈȍ~�ۑ�) */
			if (cOldEventType == *(pEvent->m_pData)) {
				memcpy (p, pEvent->m_pData + 1, pEvent->m_lLen - 1);
				p += pEvent->m_lLen - 1;
			}
			/* �����j���O�X�e�[�^�X�g�p�s�\(�S�o�C�g�ۑ�) */
			else {
				memcpy (p, pEvent->m_pData, pEvent->m_lLen);
				p += pEvent->m_lLen;
			}
			cOldEventType = *(pEvent->m_pData);

		}
		/* Sysx�C�x���g */
		else if (pEvent->m_lKind == 0xF0) {
			*p++ = (unsigned char)0xF0;
			p += LongToExpand (pEvent->m_lLen - 1, p);
			if (pEvent->m_pData != NULL && pEvent->m_lLen > 1) { /* 20091024�������ǉ� */
				memcpy (p, pEvent->m_pData + 1, pEvent->m_lLen - 1);
			}
			p += pEvent->m_lLen - 1;
			cOldEventType = 0xF0;
		}
		/* Sysx�C�x���g(����) */
		else if (pEvent->m_lKind == 0xF7) {
			/* ���f�o�b�O */
			*p++ = (unsigned char)0xF7;
			p += LongToExpand (pEvent->m_lLen, p);
			if (pEvent->m_pData != NULL && pEvent->m_lLen > 0) { /* 20091024�������ǉ� */
				memcpy (p, pEvent->m_pData, pEvent->m_lLen);
			}
			p += pEvent->m_lLen;
			cOldEventType = 0xF7;
		}
	}

	return (p - pBuf);
}


/* MIDI�f�[�^���X�^���_�[�hMIDI�t�@�C��(SMF)�Ƃ��ĕۑ� */
long __stdcall MIDIData_SaveAsSMF (MIDIData* pMIDIData, const char* pszFileName) {
	unsigned char* pBuf;
	unsigned char byBuf[256];
	long i = 0;
	long lLen, lWriteLen;
	short sNumTrack;
	MIDITrack* pMIDITrack;
	FILE* pFile;
	long lXFVersion = MIDIData_GetXFVersion (pMIDIData);

	pFile = fopen (pszFileName, "wb");
	if (pFile == NULL) {
		return 0;
	}

	/* MThd�w�b�_�[�̕ۑ� */
	memset (byBuf, 0, 256);
	strncpy ((char*)byBuf, "MThd", 4);
	pBuf = byBuf;
	LittleToBig4 (6, pBuf + 4);
	LittleToBig2 ((short)(pMIDIData->m_lFormat), pBuf + 8);
	sNumTrack = pMIDIData->m_lFormat == 0 ? 1 : (short)(pMIDIData->m_lNumTrack);
	LittleToBig2 (sNumTrack, pBuf + 10);
	LittleToBig2 ((short)(pMIDIData->m_lTimeBase), pBuf + 12);
	lWriteLen = fwrite (byBuf, 1, 14, pFile);
	if (lWriteLen < 14) {
		fclose (pFile);
		return 0;
	}

	/* �e�g���b�N�̕ۑ� */
	forEachTrack (pMIDIData, pMIDITrack) {
		lLen = MIDITrack_GuessTrackDataLenAsSMF (pMIDITrack);
		pBuf = malloc (8 + lLen);
		if (pBuf == NULL) {
			fclose (pFile);
			return 0;
		}
		lLen = MIDITrack_SaveAsSMF (pMIDITrack, pBuf + 8);
		if (lLen <= 0) {
			free (pBuf);
			fclose (pFile);
			return 0;
		}
		LittleToBig4 (lLen ,pBuf + 4);

		if (pMIDIData->m_lFormat == 0 && lXFVersion != 0 && i == 1) {
			strncpy ((char*)pBuf, "XFIH", 4); 
		}
		else if (pMIDIData->m_lFormat == 0 && lXFVersion != 0 && i == 2) {
			strncpy ((char*)pBuf, "XFKM", 4); 
		}
		else {
			strncpy ((char*)pBuf, "MTrk", 4); 
		}

		lWriteLen = fwrite (pBuf, 1, 8 + lLen, pFile);
		if (lWriteLen < 8 + lLen) {
			free (pBuf);
			fclose (pFile);
			return 0;
		}
		free (pBuf);
		i++;
	}

	fclose (pFile);
	return 1;
}



/* MIDIData���e�L�X�g�t�@�C������ǂݍ��݁A */
/* �V����MIDI�f�[�^�ւ̃|�C���^��Ԃ�(���s��NULL) */
MIDIData* __stdcall MIDIData_LoadFromText (const char* pszFileName) {
	MIDIData* pMIDIData = NULL;
	MIDITrack* pMIDITrack = NULL;
	MIDIEvent* pMIDIEvent = NULL;
	long lFormat;
	long lNumTrack;
	long lTimeBase;
	long lTimeMode;
	long lTimeResolution;
	long lNumEvent;
	long lIndex;
	long lTime;
	long lKind;
	long lLen;
	unsigned char ucData[1024];
	long lPrevCombinedEvent;
	long lNextCombinedEvent;
	long lUser1;
	long lUser2;
	long lUser3;
	long lUserFlag;
	char szType[256];
	char szTextLine[2048];
	long i = 0;
	long j = 0;
	long k = 0;

	FILE* pFile = fopen (pszFileName, "rt");
	if (pFile == NULL) {
		return 0;
	}

	memset (szTextLine, 0, sizeof (szTextLine));
	fgets (szTextLine, sizeof (szTextLine) - 1, pFile);
	if (strncmp (szTextLine, "MDat", 4) == 0) {
		fclose (pFile);
		return NULL;
	}
	sscanf (szTextLine, "%s %lX %lX %lX %lX %lX %lX %lX\n",
		szType, &lFormat, &lNumTrack, &lTimeBase, 
		&lUser1, &lUser2, &lUser3, &lUserFlag);
	if (lFormat < 0 || lFormat >= 2) {
		fclose (pFile);
 		return NULL;
	}
	if (lTimeBase & 0x00008000) {
		lTimeMode = 256 - ((lTimeBase & 0x0000FF00) >> 8);
		lTimeResolution = lTimeBase & 0x000000FF;
	}
	else {
		lTimeMode =  MIDIDATA_TPQNBASE;
		lTimeResolution = lTimeBase & 0x00007FFF;
	}
	if (lTimeMode != MIDIDATA_TPQNBASE &&
		lTimeMode != MIDIDATA_SMPTE24BASE &&
		lTimeMode != MIDIDATA_SMPTE25BASE &&
		lTimeMode != MIDIDATA_SMPTE29BASE &&
		lTimeMode != MIDIDATA_SMPTE30BASE) {
		fclose (pFile);
		return NULL;
	}
	if (lTimeResolution <= 0) {
		fclose (pFile);
		return NULL;
	}

	pMIDIData = MIDIData_Create (lFormat, lNumTrack, lTimeMode, lTimeResolution);
	if (pMIDIData == NULL) {
		fclose (pFile);
		return NULL;
	}
	pMIDIData->m_lUser1 = lUser1;
	pMIDIData->m_lUser2 = lUser2;
	pMIDIData->m_lUser3 = lUser3;
	pMIDIData->m_lUserFlag = lUserFlag;

	forEachTrack (pMIDIData, pMIDITrack) {
		memset (szTextLine, 0, sizeof (szTextLine));
		fgets (szTextLine, sizeof (szTextLine) - 1, pFile);
		if (strncmp (szTextLine, "MTrk", 9) != 0) {
			fclose (pFile);
			MIDIData_Delete (pMIDIData);
			return NULL;
		}
		sscanf (szTextLine, "%s %lX %lX %lX %lX %lX %lX\n",
			szType, &lIndex, &lNumEvent,
			&lUser1, &lUser2, &lUser3, &lUserFlag);
		pMIDITrack->m_lTempIndex = lIndex;
		pMIDITrack->m_lUser1 = lUser1;
		pMIDITrack->m_lUser2 = lUser2;
		pMIDITrack->m_lUser3 = lUser3;
		pMIDITrack->m_lUserFlag = lUserFlag;
		/* �eMIDI�C�x���g�̓ǂݍ��� */
		for (j = 0; j < lNumEvent; j++) {
			char* p = szTextLine;
			memset (szTextLine, 0, sizeof (szTextLine));
			fgets (szTextLine, sizeof (szTextLine) - 1, pFile);
			if (strncmp (szTextLine, "MEvt", 4) != 0) {
				fclose (pFile);
				MIDIData_Delete (pMIDIData);
				return NULL;
			}
			sscanf (szTextLine, "%s %lX %lX %lX %lX",
				szType, &lIndex, &lTime, &lKind, &lLen);
			memset (ucData, 0, sizeof (ucData));
			for (k = 0; k < 5; k++) {
				p = strchr (p, ' ');
				if (p == NULL) {
					fclose (pFile);
					MIDIData_Delete (pMIDIData);
					return NULL;
				}
				while (*p == ' ') {
					p++;
				}
			}
			for (k = 0; k < lLen; k++) {
				sscanf (p, "%X", &ucData[k]);
				p = strchr (p, ' ');
				if (p == NULL) {
					fclose (pFile);
					MIDIData_Delete (pMIDIData);
					return NULL;
				}
				while (*p == ' ') {
					p++;
				}
			}

			pMIDIEvent = MIDIEvent_Create (lTime, lKind, ucData, lLen);
			if (pMIDIEvent == NULL) {
				fclose (pFile);
				MIDIData_Delete (pMIDIData);
				return NULL;
			}
			if (MIDITrack_InsertEvent (pMIDITrack, pMIDIEvent) == 0) {
				fclose (pFile);
				MIDIData_Delete (pMIDIData);
				return NULL;
			}
			sscanf (p, "%lX %lX %lX %lX %lX %lX\n", 
				&lPrevCombinedEvent, &lNextCombinedEvent,
				&lUser1, &lUser2, &lUser3, &lUserFlag);
			pMIDIEvent->m_lTempIndex = lIndex;
			pMIDIEvent->m_pPrevCombinedEvent = (MIDIEvent*)lPrevCombinedEvent;
			pMIDIEvent->m_pNextCombinedEvent = (MIDIEvent*)lNextCombinedEvent;
			pMIDIEvent->m_lUser1 = lUser1;
			pMIDIEvent->m_lUser2 = lUser2;
			pMIDIEvent->m_lUser3 = lUser3;
			pMIDIEvent->m_lUserFlag = lUserFlag;
		}
		/* �������ꂽ�C�x���g�̌�����ԕ������� */
		forEachEvent (pMIDITrack, pMIDIEvent) {
			if ((long)(pMIDIEvent->m_pPrevCombinedEvent) == -1) {
				pMIDIEvent->m_pPrevCombinedEvent = NULL;
			}
			else {
				MIDIEvent* pTempEvent;
				forEachEvent (pMIDITrack, pTempEvent) {
					if (pTempEvent->m_lTempIndex ==
						(long)(pMIDIEvent->m_pPrevCombinedEvent)) {
						pMIDIEvent->m_pPrevCombinedEvent = pTempEvent;
						break;
					}
				}
				if (pTempEvent == NULL) {
					fclose (pFile);
					MIDIData_Delete (pMIDIData);
					return NULL;
				}
			}
			if ((long)(pMIDIEvent->m_pNextCombinedEvent) == -1) {
				pMIDIEvent->m_pNextCombinedEvent = NULL;
			}
			else {
				MIDIEvent* pTempEvent;
				forEachEvent (pMIDITrack, pTempEvent) {
					if (pTempEvent->m_lTempIndex ==
						(long)(pMIDIEvent->m_pNextCombinedEvent)) {
						pMIDIEvent->m_pNextCombinedEvent = pTempEvent;
						break;
					}
				}
				if (pTempEvent == NULL) {
					fclose (pFile);
					MIDIData_Delete (pMIDIData);
					return NULL;
				}
			}
		}
		i++;
	}
	return pMIDIData;
}

/* MIDIData���e�L�X�g�t�@�C���Ƃ��ĕۑ� */
long __stdcall MIDIData_SaveAsText (MIDIData* pMIDIData, const char* pszFileName) {
	MIDITrack* pMIDITrack = NULL;
	MIDIEvent* pMIDIEvent = NULL;
	long i = 0;
	long j = 0;
	long k = 0;
	FILE* pFile = fopen (pszFileName, "wt");
	if (pFile == NULL) {
		return 0;
	}
	MIDIData_CountTrack (pMIDIData),
	fprintf (pFile, "MDat %lX %lX %lX %lX %lX %lX %lX\n",
		pMIDIData->m_lFormat,
		pMIDIData->m_lNumTrack,
		pMIDIData->m_lTimeBase,
		pMIDIData->m_lUser1,
		pMIDIData->m_lUser2,
		pMIDIData->m_lUser3,
		pMIDIData->m_lUserFlag);
	forEachTrack (pMIDIData, pMIDITrack) {
		MIDITrack_CountEvent (pMIDITrack),
		fprintf (pFile, "MTrk %lX %lX %lX %lX %lX %lX\n",
			pMIDITrack->m_lTempIndex,
			pMIDITrack->m_lNumEvent,
			pMIDITrack->m_lUser1,
			pMIDITrack->m_lUser2,
			pMIDITrack->m_lUser3,
			pMIDITrack->m_lUserFlag);
		j = 0;
		forEachEvent (pMIDITrack, pMIDIEvent) {
			fprintf (pFile, "MEvt %lX %lX %lX %lX",
				pMIDIEvent->m_lTempIndex,
				pMIDIEvent->m_lTime,
				pMIDIEvent->m_lKind,
				pMIDIEvent->m_lLen);
			for (k = 0; k < pMIDIEvent->m_lLen; k++) {
				fprintf (pFile, " %X", *(pMIDIEvent->m_pData + k));
			}
			fprintf (pFile, " %lX %lX %lX %lX %lX %lX\n",
				pMIDIEvent->m_pPrevCombinedEvent ? pMIDIEvent->m_pPrevCombinedEvent->m_lTempIndex : -1,
				pMIDIEvent->m_pNextCombinedEvent ? pMIDIEvent->m_pNextCombinedEvent->m_lTempIndex : -1,
				pMIDIEvent->m_lUser1,
				pMIDIEvent->m_lUser2,
				pMIDIEvent->m_lUser3,
				pMIDIEvent->m_lUserFlag);
			j++;
		}
		i++;
	}
	fclose (pFile);
	return 1;
}



/* MIDIData���o�C�i���t�@�C������ǂݍ��݁A */
/* �V����MIDI�f�[�^�ւ̃|�C���^��Ԃ�(���s��NULL) */
MIDIData* __stdcall MIDIData_LoadFromBinary (const char* pszFileName) {
	char cMDat[4] = {"MDat"};
	char cMDa2[4] = {"MDa2"};
	char cMTrk[4] = {"MTrk"};
	char cMTr2[4] = {"MTr2"};
	char cMEvt[4] = {"MEvt"};
	MIDIData* pMIDIData = NULL;
	MIDITrack* pMIDITrack = NULL;
	MIDIEvent* pMIDIEvent = NULL;
	long lRet = 0;
	long lFormat = 0;
	long lNumTrack = 0;
	long lTimeBase = 0;
	long lTimeMode = 0;
	long lTimeResolution = 0;
	long lNumEvent = 0;
	long lInputOn = 1;
	long lInputPort = 0;
	long lInputChannel = 0;
	long lOutputOn = 1;
	long lOutputPort = 0;
	long lOutputChannel = 0;
	long lTimePlus = 0;
	long lKeyPlus = 0;
	long lVelocityPlus = 0;
	long lViewMode = 0;
	long lForeColor = 0x00000000;
	long lBackColor = 0x00FFFFFF;
	long lIndex = 0;
	long lTime = 0;
	long lKind = 0;
	long lLen = 0;
	unsigned char ucData[2048];
	long lPrevCombinedEvent = 0;
	long lNextCombinedEvent = 0;
	long lReserved1 = 0;
	long lReserved2 = 0;
	long lReserved3 = 0;
	long lReserved4 = 0;
	long lUser1 = 0;
	long lUser2 = 0;
	long lUser3 = 0;
	long lUserFlag = 0;
	char szType[4] = {0, 0, 0, 0};
	long i = 0;
	long j = 0;

	FILE* pFile = fopen (pszFileName, "rb");
	if (pFile == NULL) {
		return NULL;
	}
	/* MIDIData�ɂ��ăv���p�e�B�ǂݍ��� */
	lRet = fread (szType, 4, 1, pFile);
	if (lRet < 1 || (memcmp (szType, cMDat, 4) != 0 && memcmp (szType, cMDa2, 4) != 0)) {
		fclose (pFile);
		return NULL;
	}
	lRet = fread (&lFormat, 4, 1, pFile);
	if (lRet < 1 || lFormat < 0 || lFormat > 2) {
		fclose (pFile);
		return NULL;
	}
	lRet = fread (&lNumTrack, 4, 1, pFile);
	if (lRet < 1 || lNumTrack < 0) {
		fclose (pFile);
		return NULL;
	}
	lRet = fread (&lTimeBase, 4, 1, pFile);
	if (lRet < 1) {
		fclose (pFile);
		return NULL;
	}
	/* MIDIDataLib1.4����ǉ���������(20081008�ǉ�) */
	if (memcmp (szType, cMDa2, 4) == 0) {
		lRet = fread (&lReserved1, 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return NULL;
		}
		lRet = fread (&lReserved2, 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return NULL;
		}
		lRet = fread (&lReserved3, 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return NULL;
		}
		lRet = fread (&lReserved4, 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return NULL;
		}
	}
	lRet = fread (&lUser1, 4, 1, pFile);
	if (lRet < 1) {
		fclose (pFile);
		return NULL;
	}
	lRet = fread (&lUser2, 4, 1, pFile);
	if (lRet < 1) {
		fclose (pFile);
		return NULL;
	}
	lRet = fread (&lUser3, 4, 1, pFile);
	if (lRet < 1) {
		fclose (pFile);
		return NULL;
	}
	lRet = fread (&lUserFlag, 4, 1, pFile);
	if (lRet < 1) {
		fclose (pFile);
		return NULL;
	}
	if (lFormat < 0 || lFormat > 2) {
		fclose (pFile);
		return NULL;
	}
	if (lTimeBase & 0x00008000) {
		lTimeMode = 256 - ((lTimeBase & 0x0000FF00) >> 8);
		lTimeResolution = lTimeBase & 0x000000FF;
	}
	else {
		lTimeMode =  MIDIDATA_TPQNBASE;
		lTimeResolution = lTimeBase & 0x00007FFF;
	}
	if (lTimeMode != MIDIDATA_TPQNBASE &&
		lTimeMode != MIDIDATA_SMPTE24BASE &&
		lTimeMode != MIDIDATA_SMPTE25BASE &&
		lTimeMode != MIDIDATA_SMPTE29BASE &&
		lTimeMode != MIDIDATA_SMPTE30BASE) {
		fclose (pFile);
		return NULL;
	}
	if (lTimeResolution <= 0) {
		fclose (pFile);
		return NULL;
	}
	/* MIDIData�̐����y�уv���p�e�B�ݒ� */
	pMIDIData = MIDIData_Create (lFormat, lNumTrack, lTimeMode, lTimeResolution);
	if (pMIDIData == NULL) {
		fclose (pFile);
		return NULL;
	}
	/* MIDIDataLib1.4����ǉ���������(20081008�ǉ�) */
	if (memcmp (szType, cMDa2, 4) == 0) {
		pMIDIData->m_lReserved1 = lReserved1;
		pMIDIData->m_lReserved2 = lReserved2;
		pMIDIData->m_lReserved3 = lReserved3;
		pMIDIData->m_lReserved4 = lReserved4;
	}
	/* MIDIDataLib1.3�ȑO(20081008�ǉ�) */
	else {
		pMIDIData->m_lReserved1 = 0;
		pMIDIData->m_lReserved2 = 0;
		pMIDIData->m_lReserved3 = 0;
		pMIDIData->m_lReserved4 = 0;
	}
	pMIDIData->m_lUser1 = lUser1;
	pMIDIData->m_lUser2 = lUser2;
	pMIDIData->m_lUser3 = lUser3;
	pMIDIData->m_lUserFlag = lUserFlag;

	/* �eMIDITrack�ɂ��� */
	forEachTrack (pMIDIData, pMIDITrack) {
		/* �v���p�e�B�ǂݍ��݁E�ݒ� */
		lRet = fread (szType, 4, 1, pFile);
		if (lRet < 1 || (memcmp (szType, cMTrk, 4) != 0 && memcmp (szType, cMTr2, 4) != 0)) {
			fclose (pFile);
			return NULL;
		}
		lRet = fread (&lIndex, 4, 1, pFile);
		if (lRet < 1 || lIndex < 0) {
			fclose (pFile);
			return NULL;
		}
		lRet = fread (&lNumEvent, 4, 1, pFile);
		if (lRet < 1 || lNumEvent < 0) {
			fclose (pFile);
			return NULL;
		}
		/* MIDIDataLib1.4����ǉ���������(20081008�ǉ�) */
		if (memcmp (szType, cMTr2, 4) == 0) {
			lRet = fread (&lInputOn, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lInputPort, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lInputChannel, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lOutputOn, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lOutputPort, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lOutputChannel, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lTimePlus, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lKeyPlus, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lVelocityPlus, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lViewMode, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lForeColor, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lBackColor, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lReserved1, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lReserved2, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lReserved3, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lReserved4, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return NULL;
			}
		}
		lRet = fread (&lUser1, 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return NULL;
		}
		lRet = fread (&lUser2, 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return NULL;
		}
		lRet = fread (&lUser3, 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return NULL;
		}
		lRet = fread (&lUserFlag, 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return NULL;
		}
		pMIDITrack->m_lTempIndex = lIndex;
		pMIDITrack->m_lUser1 = lUser1;
		pMIDITrack->m_lUser2 = lUser2;
		pMIDITrack->m_lUser3 = lUser3;
		pMIDITrack->m_lUserFlag = lUserFlag;
		/* MIDIDataLib1.4����ǉ���������(20081008�ǉ�) */
		if (memcmp (szType, cMTr2, 4) == 0) {
			pMIDITrack->m_lInputOn = lInputOn;
			pMIDITrack->m_lInputPort = lInputPort;
			pMIDITrack->m_lInputChannel = lInputChannel;
			pMIDITrack->m_lOutputOn = lOutputOn;
			pMIDITrack->m_lOutputPort = lOutputPort;
			pMIDITrack->m_lOutputChannel = lOutputChannel;
			pMIDITrack->m_lTimePlus = lTimePlus;
			pMIDITrack->m_lKeyPlus = lKeyPlus;
			pMIDITrack->m_lVelocityPlus = lVelocityPlus;
			pMIDITrack->m_lViewMode = lViewMode;
			pMIDITrack->m_lForeColor = lForeColor;
			pMIDITrack->m_lBackColor = lBackColor;
			pMIDITrack->m_lReserved1 = lReserved1;
			pMIDITrack->m_lReserved2 = lReserved2;
			pMIDITrack->m_lReserved3 = lReserved3;
			pMIDITrack->m_lReserved4 = lReserved4;
		}
		/* MIDIDataLib1.3�ȑO(20081008�ǉ�) */
		else {
			pMIDITrack->m_lInputOn = (lUserFlag & 0x00000010) ? 1 : 0;
			pMIDITrack->m_lInputPort = lUser2 & 0x000000FF;
			pMIDITrack->m_lInputChannel = CLIP (-1, (long)(char)((lUser2 & 0x0000FF00) >> 8), 15);
			pMIDITrack->m_lOutputOn = (lUserFlag & 0x00000020) ? 1 : 0;
			pMIDITrack->m_lOutputPort = (lUser2 & 0x00FF0000) >> 16;
			pMIDITrack->m_lOutputChannel = CLIP (-1, (long)(char)((lUser2 & 0xFF000000) >> 24), 15);
			pMIDITrack->m_lTimePlus = 0;
			pMIDITrack->m_lKeyPlus = 0;
			pMIDITrack->m_lVelocityPlus = 0;
			pMIDITrack->m_lViewMode = (lUserFlag & 0x00000008) ? 1 : 0;
			pMIDITrack->m_lForeColor = lUser1;
			pMIDITrack->m_lBackColor = 0x00FFFFFF;
			pMIDITrack->m_lReserved1 = 0;
			pMIDITrack->m_lReserved2 = 0;
			pMIDITrack->m_lReserved3 = 0;
			pMIDITrack->m_lReserved4 = 0;
		}
		/* �eMIDIEvent�ɂ��� */
		for (j = 0; j < lNumEvent; j++) {
			/* �v���p�e�B�ǂݍ��� */
			lRet = fread (szType, 4, 1, pFile);
			if (lRet < 1 || memcmp (szType, cMEvt, 4) != 0) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lIndex, 4, 1, pFile);
			if (lRet < 1 || lIndex < 0) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lTime, 4, 1, pFile);
			if (lRet < 1 || lTime < 0) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lKind, 4, 1, pFile);
			if (lRet < 1 || lKind < 0) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lLen, 4, 1, pFile);
			if (lRet < 1 || lLen < 0 || lLen >= sizeof (ucData)) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (ucData, 1, lLen, pFile);
			if (lRet < lLen) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lPrevCombinedEvent, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lNextCombinedEvent, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lUser1, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lUser2, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lUser3, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return NULL;
			}
			lRet = fread (&lUserFlag, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return NULL;
			}
			/* MIDI�C�x���g�̐��� */
			pMIDIEvent = MIDIEvent_Create (lTime, lKind, ucData, lLen);
			if (pMIDIEvent == NULL) {
				fclose (pFile);
				MIDIData_Delete (pMIDIData);
				return NULL;
			}
			/* MIDI�g���b�N��MIDI�C�x���g��}�� */
			if (MIDITrack_AddSingleEventForce (pMIDITrack, pMIDIEvent) == 0) { // 20090712�C��
				fclose (pFile);
				MIDIData_Delete (pMIDIData);
				return NULL;
			}
			/* �v���p�e�B�ݒ� */
			pMIDIEvent->m_lTempIndex = lIndex;
			pMIDIEvent->m_pPrevCombinedEvent = (MIDIEvent*)lPrevCombinedEvent;
			pMIDIEvent->m_pNextCombinedEvent = (MIDIEvent*)lNextCombinedEvent;			
			pMIDIEvent->m_lUser1 = lUser1;
			pMIDIEvent->m_lUser2 = lUser2;
			pMIDIEvent->m_lUser3 = lUser3;
			pMIDIEvent->m_lUserFlag = lUserFlag;
		}
		/* �������ꂽ�C�x���g�̌�����ԕ������� */
		forEachEvent (pMIDITrack, pMIDIEvent) {
			if ((long)(pMIDIEvent->m_pPrevCombinedEvent) == -1) {
				pMIDIEvent->m_pPrevCombinedEvent = NULL;
			}
			else {
				MIDIEvent* pTempEvent;
				forEachEvent (pMIDITrack, pTempEvent) {
					if (pTempEvent->m_lTempIndex ==
						(long)(pMIDIEvent->m_pPrevCombinedEvent)) {
						pMIDIEvent->m_pPrevCombinedEvent = pTempEvent;
						break;
					}
				}
				if (pTempEvent == NULL) {
					fclose (pFile);
					MIDIData_Delete (pMIDIData);
					return NULL;
				}
			}
			if ((long)(pMIDIEvent->m_pNextCombinedEvent) == -1) {
				pMIDIEvent->m_pNextCombinedEvent = NULL;
			}
			else {
				MIDIEvent* pTempEvent;
				forEachEvent (pMIDITrack, pTempEvent) {
					if (pTempEvent->m_lTempIndex ==
						(long)(pMIDIEvent->m_pNextCombinedEvent)) {
						pMIDIEvent->m_pNextCombinedEvent = pTempEvent;
						break;
					}
				}
				if (pTempEvent == NULL) {
					fclose (pFile);
					MIDIData_Delete (pMIDIData);
					return NULL;
				}
			}
		}
		i++;
	}
	return pMIDIData;
}

/* MIDIData���o�C�i���t�@�C���ɕۑ� */
long __stdcall MIDIData_SaveAsBinary (MIDIData* pMIDIData, const char* pszFileName) {
	char cMDat[4] = {"MDa2"};
	char cMTr2[4] = {"MTr2"};
	char cMEvt[4] = {"MEvt"};
	MIDITrack* pMIDITrack = NULL;
	MIDIEvent* pMIDIEvent = NULL;
	long lRet = 0;
	long lTrackCount = 0;
	long lEventCount = 0;
	long i = 0;
	long j = 0;
	long lNullIndex = -1;
	FILE* pFile = fopen (pszFileName, "wb");
	if (pFile == NULL) {
		return 0;
	}
	/* MIDIData�ɂ��� */
	lTrackCount = MIDIData_CountTrack (pMIDIData);
	lRet = fwrite (cMDat, 4, 1, pFile);
	if (lRet < 1) {
		fclose (pFile);
		return 0;
	}
	lRet = fwrite (&(pMIDIData->m_lFormat), 4, 1, pFile);
	if (lRet < 1) {
		fclose (pFile);
		return 0;
	}
	lRet = fwrite (&(pMIDIData->m_lNumTrack), 4, 1, pFile);
	if (lRet < 1) {
		fclose (pFile);
		return 0;
	}
	lRet = fwrite (&(pMIDIData->m_lTimeBase), 4, 1, pFile);
	if (lRet < 1) {
		fclose (pFile);
		return 0;
	}
	lRet = fwrite (&(pMIDIData->m_lReserved1), 4, 1, pFile);
	if (lRet < 1) {
		fclose (pFile);
		return 0;
	}
	lRet = fwrite (&(pMIDIData->m_lReserved2), 4, 1, pFile);
	if (lRet < 1) {
		fclose (pFile);
		return 0;
	}
	lRet = fwrite (&(pMIDIData->m_lReserved3), 4, 1, pFile);
	if (lRet < 1) {
		fclose (pFile);
		return 0;
	}
	lRet = fwrite (&(pMIDIData->m_lReserved4), 4, 1, pFile);
	if (lRet < 1) {
		fclose (pFile);
		return 0;
	}
	lRet = fwrite (&(pMIDIData->m_lUser1), 4, 1, pFile);
	if (lRet < 1) {
		fclose (pFile);
		return 0;
	}
	lRet = fwrite (&(pMIDIData->m_lUser2), 4, 1, pFile);
	if (lRet < 1) {
		fclose (pFile);
		return 0;
	}
	lRet = fwrite (&(pMIDIData->m_lUser3), 4, 1, pFile);
	if (lRet < 1) {
		fclose (pFile);
		return 0;
	}
	lRet = fwrite (&(pMIDIData->m_lUserFlag), 4, 1, pFile);
	if (lRet < 1) {
		fclose (pFile);
		return 0;
	}
	/* �e�g���b�N�ɂ��� */
	forEachTrack (pMIDIData, pMIDITrack) {
		lEventCount = MIDITrack_CountEvent (pMIDITrack);
		lRet = fwrite (cMTr2, 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		lRet = fwrite (&(pMIDITrack->m_lTempIndex), 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		lRet = fwrite (&(pMIDITrack->m_lNumEvent), 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		lRet = fwrite (&(pMIDITrack->m_lInputOn), 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		lRet = fwrite (&(pMIDITrack->m_lInputPort), 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		lRet = fwrite (&(pMIDITrack->m_lInputChannel), 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		lRet = fwrite (&(pMIDITrack->m_lOutputOn), 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		lRet = fwrite (&(pMIDITrack->m_lOutputPort), 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		lRet = fwrite (&(pMIDITrack->m_lOutputChannel), 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		lRet = fwrite (&(pMIDITrack->m_lTimePlus), 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		lRet = fwrite (&(pMIDITrack->m_lKeyPlus), 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		lRet = fwrite (&(pMIDITrack->m_lVelocityPlus), 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		lRet = fwrite (&(pMIDITrack->m_lViewMode), 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		lRet = fwrite (&(pMIDITrack->m_lForeColor), 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		lRet = fwrite (&(pMIDITrack->m_lBackColor), 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		lRet = fwrite (&(pMIDITrack->m_lReserved1), 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		lRet = fwrite (&(pMIDITrack->m_lReserved2), 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		lRet = fwrite (&(pMIDITrack->m_lReserved3), 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		lRet = fwrite (&(pMIDITrack->m_lReserved4), 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		lRet = fwrite (&(pMIDITrack->m_lUser1), 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		lRet = fwrite (&(pMIDITrack->m_lUser2), 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		lRet = fwrite (&(pMIDITrack->m_lUser3), 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		lRet = fwrite (&(pMIDITrack->m_lUserFlag), 4, 1, pFile);
		if (lRet < 1) {
			fclose (pFile);
			return 0;
		}
		/* �e�C�x���g�ɂ��� */
		j = 0;
		forEachEvent (pMIDITrack, pMIDIEvent) {
			lRet = fwrite (cMEvt, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return 0;
			}
			lRet = fwrite (&(pMIDIEvent->m_lTempIndex), 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return 0;
			}
			lRet = fwrite (&(pMIDIEvent->m_lTime), 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return 0;
			}
			lRet = fwrite (&(pMIDIEvent->m_lKind), 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return 0;
			}
			lRet = fwrite (&(pMIDIEvent->m_lLen), 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return 0;
			}
			if (pMIDIEvent->m_pData != NULL && pMIDIEvent->m_lLen > 0) { /* 20091024�������ǉ� */
				lRet = fwrite (pMIDIEvent->m_pData, 1, pMIDIEvent->m_lLen, pFile);
				if (lRet < pMIDIEvent->m_lLen) {
					fclose (pFile);
					return 0;
				}
			}
			lRet = fwrite (pMIDIEvent->m_pPrevCombinedEvent ? 
				&(pMIDIEvent->m_pPrevCombinedEvent->m_lTempIndex) : &lNullIndex, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return 0;
			}
			lRet = fwrite (pMIDIEvent->m_pNextCombinedEvent ? 
				&(pMIDIEvent->m_pNextCombinedEvent->m_lTempIndex) : &lNullIndex, 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return 0;
			}
			lRet = fwrite (&(pMIDIEvent->m_lUser1), 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return 0;
			}
			lRet = fwrite (&(pMIDIEvent->m_lUser2), 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return 0;
			}
			lRet = fwrite (&(pMIDIEvent->m_lUser3), 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return 0;
			}
			lRet = fwrite (&(pMIDIEvent->m_lUserFlag), 4, 1, pFile);
			if (lRet < 1) {
				fclose (pFile);
				return 0;
			}
			j++;
		}
		i++;
	}
	fclose (pFile);
	return 1;
}

/* MIDITrack��Cherry�`���̃������u���b�N����ǂݍ���(����J) */
/* �V����MIDI�g���b�N�ւ̃|�C���^��Ԃ�(���s��NULL) */
MIDITrack* __stdcall MIDITrack_LoadFromCherry (long lTrackIndex, unsigned char*pTrackData, long lTrackLen) {
	MIDITrack* pMIDITrack = NULL;
	unsigned char* p = pTrackData;
	long lOutputPort = 0;
	long lOutputChannel = 0;
	char szTempTrackName[256];
	memset (szTempTrackName, 0, sizeof (szTempTrackName));

	/* �V�KMIDI�g���b�N�̐��� */
	pMIDITrack = MIDITrack_Create ();
	if (pMIDITrack == NULL) {
		return NULL;
	}

	pMIDITrack->m_lTempIndex = lTrackIndex;

	/* ���[�h(�ʏ�^�h����)�̎擾 */
	pMIDITrack->m_lViewMode = (long)(*(unsigned char*)(p + 12));

	/* �o�̓|�[�g�ԍ��̎擾 */
	lOutputPort = CLIP (0, (long)(*(char*)(p + 13)), 3);
	pMIDITrack->m_lOutputPort = lOutputPort;

	/* �o�̓`�����l���̎擾 */
	lOutputChannel = CLIP (-1, (long)(*(char*)(p + 14)), 15);
	pMIDITrack->m_lOutputChannel = (lTrackIndex <= 1 ? -1 : lOutputChannel);

	/* �L�[+�̎擾 */
	pMIDITrack->m_lKeyPlus = (long)(*(char*)(p + 15));

	/* �^�C��+�̎擾 */
	pMIDITrack->m_lTimePlus = (long)(*(char*)(p + 16));

	/* �x���V�e�B+�̎擾 */
	pMIDITrack->m_lVelocityPlus = (long)(*(char*)(p + 17));

	/* ���̑��̎擾 */
	pMIDITrack->m_lReserved1 = *(p + 28) | (*(p + 29) << 8) | (*(p + 30) << 16) | (*(p + 31) << 24);

	/* */
	pMIDITrack->m_lInputOn = 1;
	pMIDITrack->m_lInputPort = lOutputPort;
	pMIDITrack->m_lInputChannel = (lTrackIndex <= 1 ? -1 : lOutputChannel);

	/* �^�C�g���̎擾 */
	memcpy (szTempTrackName, pTrackData + 60, 64);
	MIDITrack_SetName (pMIDITrack, szTempTrackName);

	/* �e�C�x���g�̒ǉ����� */
	p = pTrackData + 124;
	while (p < pTrackData + lTrackLen) {
		/* �m�[�g */
		if (0x00 <= *p && *p <= 0x7F && lTrackIndex >= 2) {
			long lTime = (long)(*(long*)(p + 2));
			long lKey = (long)(*p);
			long lVel = (long)(*(unsigned short*)(p + 8));
			long lDur = (long)(*(unsigned short*)(p + 6));
			MIDITrack_InsertNote (pMIDITrack, lTime, lOutputChannel, lKey, lVel, lDur);
			p += 10;
		}
		/* �R���g���[���`�F���W���͉��z�R���g���[���`�F���W(�e���|�܂�) */
		else if (*p == 0x82) {
			long lTime = (long)(*(long*)(p + 2));
			long lNum = (long)(*(p + 1));
			long lBank = (long)(*(unsigned short*)(p + 6));
			long lVal = (long)(*(unsigned short*)(p + 8));
			unsigned char bySysx[256];
			memset (bySysx, 0, sizeof (bySysx));
			/* �R���g���[���`�F���W */
			if (0 <= lNum && lNum <= 127 && 0 <= lVal && lVal <= 127 && lTrackIndex >= 2) {
				MIDITrack_InsertControlChange (pMIDITrack, lTime, lOutputChannel, lNum, lVal);
			}
			/* �s�b�`�x���h�Z���V�e�B�r�e�B */
			else if (lNum == 0x80 && lTrackIndex >= 2) {
				lBank = (0 << 7) | 0;
				MIDITrack_InsertRPNChange (pMIDITrack, lTime, lOutputChannel, lBank, lVal);
			}
			/* �t�@�C���`���[��(TODO:�l�͕ۗ�:0x0000-0x4000-0x8000) */
			else if (lNum == 0x81 && lTrackIndex >= 2) {
				lBank = (0 << 7) | 1;
				MIDITrack_InsertRPNChange (pMIDITrack, lTime, lOutputChannel, lBank, lVal >> 8);
			}
			/* �R�[�X�`���[�� */
			else if (lNum == 0x82 && lTrackIndex >= 2) {
				lBank = (0 << 7) | 2;
				MIDITrack_InsertRPNChange (pMIDITrack, lTime, lOutputChannel, lBank, lVal);
			}
			/* �}�X�^�[�{�����[�� */
			else if (lNum == 0x83) {
				bySysx[0] = 0xF0;
				bySysx[1] = 0x7F;
				bySysx[2] = 0x7F;
				bySysx[3] = 0x04;
				bySysx[4] = 0x01;
				bySysx[5] = 0x00;
				bySysx[6] = (unsigned char)CLIP (0, lVal, 127);
				bySysx[7] = 0xF7;
				MIDITrack_InsertSysExEvent (pMIDITrack, lTime, bySysx, 8);
			}
			/* V.Rate / V.Dep / V.Del / HPF.Cutoff / E.G.Delay */
			else if (0x8C <= lNum && lNum <= 0x90 && lTrackIndex >= 2) {
				unsigned char ucBankMSB = 1;
				unsigned char ucBankLSB[5] = {8, 9, 10, 36, 100};
				lBank = (ucBankMSB << 7) | ucBankLSB[lNum - 0x8C];
				MIDITrack_InsertNRPNChange (pMIDITrack, lTime, lOutputChannel, lBank, lVal);
			}
			/* PEG InitLev / PEG AttackTime / PEG RelLev / PEG RelTime */
			else if (0x91 <= lNum && lNum <= 0x94) {
				bySysx[0] = 0xF0;
				bySysx[1] = 0x43;
				bySysx[2] = 0x10;
				bySysx[3] = 0x4C;
				bySysx[4] = 0x08;
				bySysx[5] = 0x01;
				bySysx[6] = (unsigned char)(lNum - 0x28); /* 0x69�`0x6C */
				bySysx[7] = (unsigned char)CLIP (0, lVal, 127);
				bySysx[8] = 0xF7;
				MIDITrack_InsertSysExEvent (pMIDITrack, lTime, bySysx, 9);
			}
			/* �L�[�A�t�^�[�^�b�` */
			else if (lNum == 0x97 && lTrackIndex >= 2) {
				MIDITrack_InsertKeyAftertouch (pMIDITrack, lTime, lOutputChannel, 0, lVal);
			}
			/* �s�b�`�x���h */
			else if (lNum == 0x99 && lTrackIndex >= 2) {
				MIDITrack_InsertPitchBend (pMIDITrack, lTime, lOutputChannel, lVal);
			}
			/* �e���| */
			else if (lNum == 0x9B && lTrackIndex == 0) {
				long lTempo = 60000000 / CLIP (1, lVal, 65535);
				MIDITrack_InsertTempo (pMIDITrack, lTime, lTempo);
			}
			/* GM System On */
			else if (lNum == 0xA0) {
				bySysx[0] = 0xF0;
				bySysx[1] = 0x7E;
				bySysx[2] = 0x7F;
				bySysx[3] = 0x09;
				bySysx[4] = 0x01;
				bySysx[5] = 0xF7;
				MIDITrack_InsertSysExEvent (pMIDITrack, lTime, bySysx, 6);
			}
			/* XG System On */
			/* GS Reset */
			else if (lNum == 0xA1) {
				unsigned char ucModule = (unsigned char)(pMIDITrack->m_lReserved1 & 0x000000FF);
				if (64 <= ucModule && ucModule < 96) { /* GS */
					bySysx[0] = 0xF0;
					bySysx[1] = 0x41;
					bySysx[2] = 0x10;
					bySysx[3] = 0x42;
					bySysx[4] = 0x12;
					bySysx[5] = 0x40;
					bySysx[6] = 0x00;
					bySysx[7] = 0x7F;
					bySysx[8] = 0x00;
					bySysx[9] = 128 - (Sum (&bySysx[5], 4) % 128);
					bySysx[10] = 0xF7;
					MIDITrack_InsertSysExEvent (pMIDITrack, lTime, bySysx, 11);
				}
				else { /* XG, others */
					bySysx[0] = 0xF0;
					bySysx[1] = 0x43;
					bySysx[2] = 0x10;
					bySysx[3] = 0x4C;
					bySysx[4] = 0x00;
					bySysx[5] = 0x00;
					bySysx[6] = 0x7E;
					bySysx[7] = 0x00;
					bySysx[8] = 0xF7;
					MIDITrack_InsertSysExEvent (pMIDITrack, lTime, bySysx, 9);
				}
			}
			/* 88 Reset */
			else if (lNum == 0xA2) {
				bySysx[0] = 0xF0;
				bySysx[1] = 0x41;
				bySysx[2] = 0x10;
				bySysx[3] = 0x42;
				bySysx[4] = 0x12;
				bySysx[5] = 0x00;
				bySysx[6] = 0x00;
				bySysx[7] = 0x7F;
				bySysx[8] = (unsigned char)CLIP (0, lVal, 1);
				bySysx[9] = ((128 - (Sum (&bySysx[5], 4) % 128)) & 0x7F);
				bySysx[10] = 0xF7;
				MIDITrack_InsertSysExEvent (pMIDITrack, lTime, bySysx, 11);
			}
			/* Dry Level */
			else if (lNum == 0xA5) {
				bySysx[0] = 0xF0;
				bySysx[1] = 0x43;
				bySysx[2] = 0x10;
				bySysx[3] = 0x4C;
				bySysx[4] = 0x08;
				bySysx[5] = 0x03;
				bySysx[6] = 0x11;
				bySysx[7] = (unsigned char)CLIP (0, lVal, 127);
				bySysx[8] = 0xF7;
				MIDITrack_InsertSysExEvent (pMIDITrack, lTime, bySysx, 9);
			}
			/* MW params */
			else if (0xA6 <= lNum && lNum <= 0xAB) {
				bySysx[0] = 0xF0;
				bySysx[1] = 0x43;
				bySysx[2] = 0x10;
				bySysx[3] = 0x4C;
				bySysx[4] = 0x08;
				bySysx[5] = 0x02;
				bySysx[6] = (unsigned char)(lNum - 0xA6 + 0x1D); /* 0x1D�`0x22 */
				bySysx[7] = (unsigned char)CLIP (0, lVal, 127);
				bySysx[8] = 0xF7;
				MIDITrack_InsertSysExEvent (pMIDITrack, lTime, bySysx, 9);
			}
			/* GS Reverb / Chorus / Deray Macro */
			else if (0xAC <= lNum && lNum <= 0xAE) {
				unsigned char ucType[3] = {0x30, 0x38, 0x50};
				bySysx[0] = 0xF0;
				bySysx[1] = 0x41;
				bySysx[2] = 0x10;
				bySysx[3] = 0x42;
				bySysx[4] = 0x12;
				bySysx[5] = 0x40;
				bySysx[6] = 0x01;
				bySysx[7] = ucType[lNum - 0xAC];
				bySysx[8] = (unsigned char)CLIP (0, lVal, 7);
				bySysx[9] = ((128 - (Sum (&bySysx[5], 4) % 128)) & 0x7F);
				bySysx[10] = 0xF7;
				MIDITrack_InsertSysExEvent (pMIDITrack, lTime, bySysx, 11);
			}
			/* EQ */
			else if (0xAF <= lNum && lNum <= 0xB2) {
				unsigned char ucModule = (unsigned char)(pMIDITrack->m_lReserved1 & 0x000000FF);
				if (64 <= ucModule && ucModule < 96) { /* GS */
					bySysx[0] = 0xF0;
					bySysx[1] = 0x41;
					bySysx[2] = 0x10;
					bySysx[3] = 0x42;
					bySysx[4] = 0x12;
					bySysx[5] = 0x40;
					bySysx[6] = 0x02;
					bySysx[7] = (unsigned char)(lNum - 0xAF);
					bySysx[8] = (unsigned char)CLIP (0, lVal, 127);
					bySysx[9] = ((128 - (Sum (&bySysx[5], 4) % 128)) & 0x7F);
					bySysx[10] = 0xF7;
					MIDITrack_InsertSysExEvent (pMIDITrack, lTime, bySysx, 11);
				}
				else if (lTrackIndex >= 2) { /* XG, others */
					unsigned char ucBankMSB = 1;
					unsigned char ucBankLSB[4] = {48, 49, 52, 53};
					lBank = (ucBankMSB << 7) | ucBankLSB[lNum - 0xAF];
					MIDITrack_InsertNRPNChange (pMIDITrack, lTime, lOutputChannel, lBank, lVal); 
				}
			}
			/* DX:AssignPart or Assign Mode Full */
			else if (lNum == 0xB3) {
				unsigned char ucModule = (unsigned char)(pMIDITrack->m_lReserved1 & 0x000000FF);
				if (64 <= ucModule && ucModule < 96) { /* GS */
					bySysx[0] = 0xF0;
					bySysx[1] = 0x41;
					bySysx[2] = 0x10;
					bySysx[3] = 0x42;
					bySysx[4] = 0x12;
					bySysx[5] = 0x40;
					bySysx[6] = 0x17;
					bySysx[7] = 0x14;
					bySysx[8] = (unsigned char)CLIP (0, lVal, 127);
					bySysx[9] = ((128 - (Sum (&bySysx[5], 4) % 128)) & 0x7F);
					bySysx[10] = 0xF7;
					MIDITrack_InsertSysExEvent (pMIDITrack, lTime, bySysx, 11);
				}
				else { /* XG, others */
					bySysx[0] = 0xF0;
					bySysx[1] = 0x43;
					bySysx[2] = 0x10;
					bySysx[3] = 0x4C;
					bySysx[4] = 0x70;
					bySysx[5] = 0x02;
					bySysx[6] = 0x00;
					bySysx[7] = 0x02;
					bySysx[8] = 0xF7;
					MIDITrack_InsertSysExEvent (pMIDITrack, lTime, bySysx, 9);
				}
			}
			/* GS: Use For Drum Norm/Drum1/Drum2 */
			else if (lNum == 0xB4) {
				bySysx[0] = 0xF0;
				bySysx[1] = 0x41;
				bySysx[2] = 0x10;
				bySysx[3] = 0x42;
				bySysx[4] = 0x12;
				bySysx[5] = 0x40;
				bySysx[6] = 0x17;
				bySysx[7] = 0x15;
				bySysx[8] = (unsigned char)CLIP (0, lVal, 2);
				bySysx[9] = ((128 - (Sum (&bySysx[5], 4) % 128)) & 0x7F);
				bySysx[10] = 0xF7;
				MIDITrack_InsertSysExEvent (pMIDITrack, lTime, bySysx, 11);
			}
			/* GS: EQ SW off/on */
			else if (lNum == 0xB5) {
				bySysx[0] = 0xF0;
				bySysx[1] = 0x41;
				bySysx[2] = 0x10;
				bySysx[3] = 0x42;
				bySysx[4] = 0x12;
				bySysx[5] = 0x40;
				bySysx[6] = 0x47;
				bySysx[7] = 0x20;
				bySysx[8] = (unsigned char)CLIP (0, lVal, 1);
				bySysx[9] = ((128 - (Sum (&bySysx[5], 4) % 128)) & 0x7F);
				bySysx[10] = 0xF7;
				MIDITrack_InsertSysExEvent (pMIDITrack, lTime, bySysx, 11);
			}
			/* Drum params */
			else if (0xC8 <= lNum && lNum <= 0xD7 && lTrackIndex >= 2) {
				unsigned char ucBankMSB[16] = {20, 21, 22, 23, 24, 25, 26, 28, 29, 30, 31, 36, 52, 48, 53, 49};
				unsigned char ucBankLSB = (unsigned char)(lBank & 0x007F);
				lBank = (ucBankMSB[lNum - 0xC8] << 7) | ucBankLSB;
				MIDITrack_InsertNRPNChange (pMIDITrack, lTime, lOutputChannel, lBank, lVal); 
			}
			p += 10;
		}
		/* �v���O�����`�F���W���̓p�b�`�`�F���W */
		else if (*p == 0x83 && lTrackIndex >= 2) {
			long lTime = (long)(*(long*)(p + 2));
			long lBankMSB = (long)(*(unsigned char*)(p + 6));
			long lBankLSB = (long)(*(unsigned char*)(p + 7));
			long lVal = (long)(*(unsigned short*)(p + 8));
			if (0 <= lBankMSB && lBankMSB <= 127 && 0 <= lBankLSB && lBankLSB <= 127) {
				long lBank = (lBankMSB << 7) | lBankLSB; 
				MIDITrack_InsertPatchChange (pMIDITrack, lTime, lOutputChannel, lBank, lVal);
			}
			else if (0 <= lVal && lVal <= 127) {
				if (0 <= lBankMSB && lBankMSB <= 127) {
					MIDITrack_InsertControlChange (pMIDITrack, lTime, lOutputChannel, 0, lBankMSB);
				}
				if (0 <= lBankLSB && lBankLSB <= 127) {
					MIDITrack_InsertControlChange (pMIDITrack, lTime, lOutputChannel, 32, lBankLSB);
				}
				MIDITrack_InsertProgramChange (pMIDITrack, lTime, lOutputChannel, lVal);
			}
			p += 10;
		}
		/* MIDI�|�[�g�E�`�����l���ؑ� */
		else if (*p == 0x84) {
			long lTime = (long)(*(long*)(p + 2));
			lOutputPort = (long)(*(unsigned short*)(p + 6));
			lOutputChannel = (long)(*(unsigned short*)(p + 8));
			p += 10;
		}
		/* �V�X�e���G�N�X�N���[�V�� */
		else if (*p == 0xC0) {
			long lTime = (long)(*(long*)(p + 2));
			long lLen = (long)(*(long*)(p + 10));
			MIDITrack_InsertSysExEvent (pMIDITrack, lTime, p + 14, lLen);
			p += (14 + lLen);
		}
		/* �e�L�X�g�C�x���g */
		else if (*p == 0xC1) {
			long lTime = (long)(*(long*)(p + 2));
			long lLen = (long)(*(long*)(p + 10));
			char szText[65535];
			memset (szText, 0, sizeof (szText));
			memcpy (szText, p + 14, lLen);
			MIDITrack_InsertTextEvent (pMIDITrack, lTime, szText);
			p += (14 + lLen);
		}
		/* �̎� */
		else if (*p == 0xC2) {
			long lTime = (long)(*(long*)(p + 2));
			long lLen = (long)(*(long*)(p + 10));
			char szText[65535];
			memset (szText, 0, sizeof (szText));
			memcpy (szText, p + 14, lLen);
			MIDITrack_InsertLyric (pMIDITrack, lTime, szText);
			p += (14 + lLen);
		}
		/* �G���h�I�u�g���b�N */
		else if (*p == 0xFF) {
			long lTime = (long)(*(long*)(p + 2));
			MIDITrack_InsertEndofTrack (pMIDITrack, lTime);
			p += 10;
			break;
		}
		
	}
	return pMIDITrack;
}

/* MIDIData��Cherrry�t�@�C��(chy)����ǂݍ��݁A */
/* �V����MIDI�f�[�^�ւ̃|�C���^��Ԃ�(���s��NULL) */
MIDIData* __stdcall MIDIData_LoadFromCherry (const char* pszFileName) {
	MIDIData* pMIDIData = NULL;
	MIDITrack* pMIDITrack = NULL;
	FILE* pFile = NULL;
	long lRet = 0;
	long lTrackLen = 0;
	long lTrackIndex = 0;
	unsigned char* pTrackData = NULL;
	unsigned char byBuf[256];
	char szTempTitle[256];
	char szTempCopyright[256];
	long lTempNumTrack = 0;
	long lTempExtendedDataAddress[64];
	long lTempTrackAddress[256];
	memset (szTempTitle, 0, sizeof (szTempTitle));
	memset (szTempCopyright, 0, sizeof (szTempCopyright));
	memset (lTempExtendedDataAddress, 0, sizeof (long) * 64);
	memset (lTempTrackAddress, 0, sizeof (long) * 256);

	pMIDIData = malloc (sizeof (MIDIData));
	memset (pMIDIData, 0, sizeof (MIDIData));
	if (pMIDIData == NULL) {
		return NULL;
	}
	pMIDIData->m_pFirstTrack = NULL;
	pMIDIData->m_pLastTrack = NULL;
	pMIDIData->m_pNextSeq = NULL;
	pMIDIData->m_pPrevSeq = NULL;
	pMIDIData->m_pParent = NULL;
	pMIDIData->m_lUser1 = 0;
	pMIDIData->m_lUser2 = 0;
	pMIDIData->m_lUser3 = 0;

	pFile = fopen (pszFileName, "rb");
	if (pFile == NULL) {
		free (pMIDIData);
		return NULL;
	}
	if (fread (byBuf, sizeof(char), 256, pFile) < 256) {
		free (pMIDIData);
		fclose (pFile);
		return NULL;
	}

	/* ����ID�ǂݍ��� */
	if (memcmp (byBuf, "CHRY0101", 8) != 0) {
		free (pMIDIData);
		fclose (pFile);
		return NULL; 
	}
	pMIDIData->m_lFormat = 1;

	/* �^�C���x�[�X�ǂݍ���(1�`960) */
	pMIDIData->m_lTimeBase = (long)(*(unsigned short*)(byBuf + 8));
	if (pMIDIData->m_lTimeBase < 1 || pMIDIData->m_lTimeBase > 960) {
		free (pMIDIData);
		fclose (pFile);
		return NULL;
	}
	
	/* �g���b�N���ǂݍ��� */
	lTempNumTrack = (long)(*(unsigned short*)(byBuf + 10));
	if (lTempNumTrack <= 0 || lTempNumTrack > 256) {
		free (pMIDIData);
		fclose (pFile);
		return NULL;
	}

	/* �^�C�g���ǂݍ��� */
	if (fread (szTempTitle, sizeof(char), 128, pFile) < 128) {
		free (pMIDIData);
		fclose (pFile);
		return NULL;
	}

	/* ���쌠�ǂݍ��� */
	if (fread (szTempCopyright, sizeof(char), 128, pFile) < 128) {
		free (pMIDIData);
		fclose (pFile);
		return NULL;
	}

	/* �g���f�[�^�擪�A�h���X�z��ǂݍ��� */
	if (fread (lTempExtendedDataAddress, sizeof(long), 64, pFile) < 64) {
		free (pMIDIData);
		fclose (pFile);
		return NULL;
	}

	/* �g���b�N�擪�A�h���X�z��ǂݍ��� */
	if (fread (lTempTrackAddress, sizeof(long), lTempNumTrack, pFile) < (unsigned long)lTempNumTrack) {
		free (pMIDIData);
		fclose (pFile);
		return NULL;
	}

	/* �e�g���b�N�ǂݍ��� */
	for (lTrackIndex = 0; lTrackIndex < lTempNumTrack; lTrackIndex++) {

		/* �g���b�N�̒���[�o�C�g]�擾 */
		if (fread (&lTrackLen, sizeof (long), 1, pFile) < 1) {
			free (pTrackData);
			free (pMIDIData);
			fclose (pFile);
			return NULL;
		}
		if (lTrackLen < 128 || lTrackLen >= 16777216) {
			free (pTrackData);
			free (pMIDIData);
			fclose (pFile);
			return NULL;
		}

		/* 1�g���b�N�p�̃��������蓖�� */
		pTrackData = malloc (lTrackLen - 4);
		if (pTrackData == NULL) {
			free (pTrackData);
			free (pMIDIData);
			fclose (pFile);
			return NULL;
		}
		
		/* 1�g���b�N�ǂݍ��� */
		if (fread (pTrackData, sizeof (char), lTrackLen - 4, pFile) < (unsigned long)lTrackLen - 4) {
			free (pTrackData);
			free (pMIDIData);
			fclose (pFile);
			return NULL;
		}

		/* �g���b�N���̃f�[�^�ǂݍ��݊֐����Ăяo�� */
		pMIDITrack = MIDITrack_LoadFromCherry (lTrackIndex, pTrackData, lTrackLen);
		if (pMIDITrack == NULL) {
			free (pTrackData);
			free (pMIDIData);
			fclose (pFile);
			return NULL;
		}
		
		/* �V�������ꂽ�g���b�N��ǉ� */
		lRet = MIDIData_AddTrackForce (pMIDIData, pMIDITrack);
		if (lRet == 0) {
			free (pTrackData);
			free (pMIDIData);
			fclose (pFile);
		}
	
		free (pTrackData);
		pTrackData = NULL;
	}

	/* �g���f�[�^ */
	pMIDITrack = pMIDIData->m_pFirstTrack;
	assert (pMIDITrack);
	/* �������q�}�[�J�[�f�[�^(����ꍇ�̂�) */
           	if (!feof (pFile) && lTempExtendedDataAddress[0] != 0x00000000) {
		long lExtendedLen = 0;
		long lTimeKeySignatureIndex = 0;
		if (fread (&lExtendedLen, sizeof (long), 1, pFile) < 1) {
			free (pMIDIData);
			fclose (pFile);
			return NULL;
		}
		/* 1��ɂ�80�o�C�g */
		for (lTimeKeySignatureIndex = 0; 
			lTimeKeySignatureIndex < lExtendedLen / 80; 
			lTimeKeySignatureIndex++) {
			long lMeasure = 0;
			long lTime = 0;
			long lnn = 0;
			long lddTemp = 0;
			long ldd = 2;
			long lcc = 24;
			long lbb = 8;
			long lsf = 0;
			long lmi = 0;
			long j;
			memset (byBuf, 0, sizeof (byBuf));
			if (fread (byBuf, sizeof (char), 80, pFile) < 80) {
				free (pMIDIData);
				fclose (pFile);
				return NULL;
			}
			lMeasure = (long)(*(unsigned short*)(byBuf));
			lnn = (long)(*(unsigned char*)(byBuf + 2));
			lddTemp = (long)(*(unsigned char*)(byBuf + 3));
			for (j = 0; j < 7; j++) {
				if (lddTemp == (1 << j)) {
					ldd = j;
					break;
				}
			}
 			lsf = (long)(*(unsigned char*)(byBuf + 4));
			if (lsf >= 9) {
				lsf = 8 - lsf;
			}
			byBuf[79] = 0;
			MIDIData_MakeTime (pMIDIData, lMeasure, 0, 0, &lTime);
			/* �}�[�J�[�}�� */
			MIDITrack_InsertMarker (pMIDITrack, lTime, (char*)(&byBuf[16]));
			/* �����L���}�� */
			MIDITrack_InsertKeySignature (pMIDITrack, lTime, lsf, lmi);
			/* ���q�L���}�� */
			MIDITrack_InsertTimeSignature (pMIDITrack, lTime, lnn, ldd, lcc, lbb);
		}
	}

	fclose (pFile);

	MIDIData_SetTitle (pMIDIData, szTempTitle);
	MIDIData_SetCopyright (pMIDIData, szTempCopyright);
	
	return pMIDIData;
}

/* MIDITrack��Cherry�`���Ń������u���b�N��ɕۑ������Ƃ��̒����𐄒�(����J) */
long __stdcall MIDITrack_GuessTrackDataLenAsCherry (MIDITrack* pMIDITrack) {
	MIDIEvent* pMIDIEvent = NULL;
	long lTrackLen = 128;
	assert (pMIDITrack);
	forEachEvent (pMIDITrack, pMIDIEvent) {
		if (MIDIEvent_IsTimeSignature (pMIDIEvent)) {
			lTrackLen += 80;
		}
		else if (MIDIEvent_IsKeySignature (pMIDIEvent)) {
			lTrackLen += 80;
		}
		else if (MIDIEvent_IsMarker (pMIDIEvent)) {
			lTrackLen += 80;
		}
		else if (MIDIEvent_IsMetaEvent (pMIDIEvent)) {
			lTrackLen += (15 + MIDIEvent_GetLen (pMIDIEvent));
		}
		else if (MIDIEvent_IsMIDIEvent (pMIDIEvent)) {
			lTrackLen += 10;
		}
		else if (MIDIEvent_IsSysExEvent (pMIDIEvent)) {
			lTrackLen += (14 + MIDIEvent_GetLen (pMIDIEvent));
		}
	}
	return lTrackLen;
}

/* MIDITrack��Cherry�`���Ń������u���b�N��ɕۑ�(Cherry��Ńg���b�N0)(����J) */
long __stdcall MIDITrack_SaveAsCherry0 (MIDITrack* pMIDITrack, unsigned char* pTrackData, long lTrackLen) {
	MIDIEvent* pMIDIEvent = NULL;
	unsigned char* p = pTrackData;

	assert (pMIDITrack);
	assert (pTrackData);
	assert (pMIDITrack->m_lTempIndex == 0);
	assert (0 <= lTrackLen && lTrackLen <= 0x7FFFFFFF);

	/* �\��(12byte) */
	memset (p, 0x00, 12);
	p += 12;

	/* �������f�[�^(48byte) */
	memset (p, 0xFF, 48);
	*(p + 0) = (unsigned char)(CLIP (0, pMIDITrack->m_lViewMode, 2));
	*(p + 1) = (unsigned char)(CLIP (0, pMIDITrack->m_lOutputPort, 3));
	*(p + 2) = (unsigned char)(CLIP (0, pMIDITrack->m_lOutputChannel, 15));
	*(p + 3) = (unsigned char)(CLIP (-127, pMIDITrack->m_lKeyPlus, 127));
	*(p + 4) = (unsigned char)(CLIP (-127, pMIDITrack->m_lTimePlus, 127));
	*(p + 5) = (unsigned char)(CLIP (-127, pMIDITrack->m_lVelocityPlus, 127));
	if (pMIDITrack->m_lReserved1 != 0x00000000) {
		*(p + 16) = (unsigned char)((pMIDITrack->m_lReserved1 >> 0) & 0xFF);
		*(p + 17) = (unsigned char)((pMIDITrack->m_lReserved1 >> 8) & 0xFF);
		*(p + 18) = (unsigned char)((pMIDITrack->m_lReserved1 >> 16) & 0xFF);
		*(p + 19) = (unsigned char)((pMIDITrack->m_lReserved1 >> 24) & 0xFF);
	}
	p += 48;

	/* �g���b�N��(64byte) */
	memset (p, 0x00, 64);
	strcpy ((char*)p, "Conductor Track");
	p += 64;

	/* �e���|�C�x���g�̂ݏ������� */
	forEachEvent (pMIDITrack, pMIDIEvent) {
		/* �e���| */
		if (MIDIEvent_IsTempo (pMIDIEvent)) {
			long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
			unsigned short usTempoBPM = (
				(unsigned short)CLIP (1, 60000000 / MIDIEvent_GetTempo (pMIDIEvent), 65535));
			*p = 0x82;
			*(p + 1) = 0x9B;
			memcpy (p + 2, &lTime, 4);
			memset (p + 6, 0x00, 2);
			memcpy (p + 8, &usTempoBPM, 2);
			p += 10;
		}
		/* �G���h�I�u�g���b�N */
		else if (MIDIEvent_IsEndofTrack (pMIDIEvent)) {
			long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
			*p = 0xFF;
			*(p + 1) = 0x00;
			memcpy (p + 2, &lTime, 4);
			memset (p + 6, 0x00, 2);
			memset (p + 8, 0x00, 2);
			p += 10;
		}
	}
	return (p - pTrackData);
}

/* MIDITrack��Cherry�`���Ń������u���b�N��ɕۑ�(Cherry��Ńg���b�N1)(����J) */
long __stdcall MIDITrack_SaveAsCherry1 (MIDITrack* pMIDITrack, unsigned char* pTrackData, long lTrackLen) {
	MIDIEvent* pMIDIEvent = NULL;
	unsigned char* p = pTrackData;
	unsigned char szTempBuf[256];
	memset (szTempBuf, 0, sizeof (szTempBuf));

	assert (pMIDITrack);
	assert (pTrackData);
	assert (pMIDITrack->m_lTempIndex == 0 || pMIDITrack->m_lTempIndex == 1);
	assert (0 <= lTrackLen && lTrackLen <= 0x7FFFFFFF);

	/* �\��(12byte) */
	memset (p, 0x00, 12);
	p += 12;
		
	/* �������f�[�^(48byte) */
	memset (p, 0xFF, 48);
	*(p + 0) = (unsigned char)(CLIP (0, pMIDITrack->m_lViewMode, 2));
	*(p + 1) = (unsigned char)(CLIP (0, pMIDITrack->m_lOutputPort, 3));
	*(p + 2) = (unsigned char)(CLIP (0, pMIDITrack->m_lOutputChannel, 15));
	*(p + 3) = (unsigned char)(CLIP (-127, pMIDITrack->m_lKeyPlus, 127));
	*(p + 4) = (unsigned char)(CLIP (-127, pMIDITrack->m_lTimePlus, 127));
	*(p + 5) = (unsigned char)(CLIP (-127, pMIDITrack->m_lVelocityPlus, 127));
	if (pMIDITrack->m_lReserved1 != 0x00000000) {
		*(p + 16) = (unsigned char)((pMIDITrack->m_lReserved1 >> 0) & 0xFF);
		*(p + 17) = (unsigned char)((pMIDITrack->m_lReserved1 >> 8) & 0xFF);
		*(p + 18) = (unsigned char)((pMIDITrack->m_lReserved1 >> 16) & 0xFF);
		*(p + 19) = (unsigned char)((pMIDITrack->m_lReserved1 >> 24) & 0xFF);
	}
	p += 48;

	/* �g���b�N��(64byte) */
	memset (p, 0x00, 64);
	strcpy ((char*)p, "System Setup");
	p += 64;

	/* Sysx�E�e�L�X�g�E�̎��C�x���g�̂ݏ������� */
	forEachEvent (pMIDITrack, pMIDIEvent) {
		/* �V�X�e���G�N�X�N���[�V�� */
		if (MIDIEvent_IsSysExEvent (pMIDIEvent)) {
			long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
			long lLen = 0;
			memset (szTempBuf, 0, sizeof (szTempBuf));
			lLen = MIDIEvent_GetData (pMIDIEvent, szTempBuf, sizeof (szTempBuf));
			*p = 0xC0;
			*(p + 1) = 0x00;
			memcpy (p + 2, &lTime, 4);
			memset (p + 6, 0x00, 2);
			memset (p + 8, 0x00, 2);
			memcpy (p + 10, &lLen, 4);
			memcpy (p + 14, szTempBuf, lLen);
			p += (14 + lLen);
		}
		/* �e�L�X�g */
		else if (MIDIEvent_IsTextEvent (pMIDIEvent)) {
			long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
			long lLen = 0;
			memset (szTempBuf, 0, sizeof (szTempBuf));
			lLen = MIDIEvent_GetData (pMIDIEvent, szTempBuf, sizeof (szTempBuf) - 1);
			lLen += 1;
			*p = 0xC1;
			*(p + 1) = 0x00;
			memcpy (p + 2, &lTime, 4);
			memset (p + 6, 0x00, 2);
			memset (p + 8, 0x00, 2);
			memcpy (p + 10, &lLen, 4);
			memcpy (p + 14, szTempBuf, lLen);
			p += (14 + lLen);
		}
		/* �̎� */
		else if (MIDIEvent_IsLyric (pMIDIEvent)) {
			long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
			long lLen = 0;
			memset (szTempBuf, 0, sizeof (szTempBuf));
			lLen = MIDIEvent_GetData (pMIDIEvent, szTempBuf, sizeof (szTempBuf) - 1);
			lLen += 1;
			*p = 0xC2;
			*(p + 1) = 0x00;
			memcpy (p + 2, &lTime, 4);
			memset (p + 6, 0x00, 2);
			memset (p + 8, 0x00, 2);
			memcpy (p + 10, &lLen, 4);
			memcpy (p + 14, szTempBuf, lLen);
			p += (14 + lLen);
		}
		/* �G���h�I�u�g���b�N */
		else if (MIDIEvent_IsEndofTrack (pMIDIEvent)) {
			long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
			*p = 0xFF;
			*(p + 1) = 0x00;
			memcpy (p + 2, &lTime, 4);
			memset (p + 6, 0x00, 2);
			memset (p + 8, 0x00, 2);
			p += 10;
		}
	}
	return (p - pTrackData);
}

/* MIDITrack��Cherry�`���Ń������u���b�N��ɕۑ�(Cherry��Ńg���b�N1�ȍ~)(����J) */
long __stdcall MIDITrack_SaveAsCherry2 (MIDITrack* pMIDITrack, unsigned char* pTrackData, long lTrackLen) {
	MIDIEvent* pMIDIEvent = NULL;
	unsigned char* p = pTrackData;
	unsigned char szTempBuf[256];
	memset (szTempBuf, 0, sizeof (szTempBuf));
	assert (pMIDITrack);
	assert (pTrackData);
	assert (pMIDITrack->m_lTempIndex >= 1);
	assert (0 <= lTrackLen && lTrackLen <= 0x7FFFFFFF);
		
	/* �\��̈�(12byte) */
	memset (p, 0x00, 12);
	p += 12;
	
	/* �������f�[�^(48byte) */
	memset (p, 0xFF, 48);
	*(p + 0) = (unsigned char)(CLIP (0, pMIDITrack->m_lViewMode, 2));
	*(p + 1) = (unsigned char)(CLIP (0, pMIDITrack->m_lOutputPort, 3));
	*(p + 2) = (unsigned char)(CLIP (0, pMIDITrack->m_lOutputChannel, 15));
	*(p + 3)= (unsigned char)(CLIP (-127, pMIDITrack->m_lKeyPlus, 127));
	*(p + 4) = (unsigned char)(CLIP (-127, pMIDITrack->m_lTimePlus, 127));
	*(p + 5)= (unsigned char)(CLIP (-127, pMIDITrack->m_lVelocityPlus, 127));
	if (pMIDITrack->m_lReserved1 != 0x00000000) {
		*(p + 16) = (unsigned char)((pMIDITrack->m_lReserved1 >> 0) & 0xFF);
		*(p + 17) = (unsigned char)((pMIDITrack->m_lReserved1 >> 8) & 0xFF);
		*(p + 18) = (unsigned char)((pMIDITrack->m_lReserved1 >> 16) & 0xFF);
		*(p + 19) = (unsigned char)((pMIDITrack->m_lReserved1 >> 24) & 0xFF);
	}
	p += 48;

	/* �g���b�N��(64byte) */
	memset (p, 0x00, 64);
	MIDITrack_GetName (pMIDITrack, (char*)p, 63);
	p += 64;

	/* �m�[�g�I���ECC#�EPC#�ESysx�E�e�L�X�g�E�̎��C�x���g�̂ݏ������� */
	forEachEvent (pMIDITrack, pMIDIEvent) {
		/* �m�[�g�I��(�m�[�g�I�t�Ɍ������Ă�����̂̂�) */
		if (MIDIEvent_IsNoteOn (pMIDIEvent) && pMIDIEvent->m_pNextCombinedEvent) {
			long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
			unsigned char ucKey = (unsigned char)(CLIP (0, MIDIEvent_GetKey (pMIDIEvent), 127));
			unsigned short usDur = (unsigned short)(CLIP (0, MIDIEvent_GetDuration (pMIDIEvent), 65535));
			unsigned short usVel = (unsigned short)(CLIP (0, MIDIEvent_GetVelocity (pMIDIEvent), 127));
			*p = ucKey;
			*(p + 1) = 0;
			memcpy (p + 2, &lTime, 4);
			memcpy (p + 6, &usDur, 2);
			memcpy (p + 8, &usVel, 2);
			p += 10;
		}
		/* �p�b�`�`�F���W(CC#0+CC#32+PC#) */
		else if (MIDIEvent_IsPatchChange (pMIDIEvent)) {
			long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
			unsigned short usBankMSB = (unsigned short)MIDIEvent_GetBankMSB (pMIDIEvent);
			unsigned short usBankLSB = (unsigned short)MIDIEvent_GetBankLSB (pMIDIEvent);
			unsigned short usBank16 = (usBankMSB << 8) | usBankLSB;
			unsigned short usValue = (unsigned short)MIDIEvent_GetValue 
				(pMIDIEvent->m_pNextCombinedEvent->m_pNextCombinedEvent);
			*p = 0x83;
			*(p + 1) = 0x00;
			memcpy (p + 2, &lTime, 4);
			memcpy (p + 6, &usBank16, 2);
			memcpy (p + 8, &usValue, 2);
			p += 10;
			pMIDIEvent = MIDIEvent_GetLastCombinedEvent (pMIDIEvent);
		}
		/* RPN�`�F���W(CC#101+CC#100+CC#6) */
		else if (MIDIEvent_IsRPNChange (pMIDIEvent)) {
			long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
			unsigned short usBankMSB = (unsigned short)MIDIEvent_GetBankMSB (pMIDIEvent);
			unsigned short usBankLSB = (unsigned short)MIDIEvent_GetBankLSB (pMIDIEvent);
			unsigned short usDummy = 0x0000;
			unsigned short usValue = (unsigned short)MIDIEvent_GetValue 
				(pMIDIEvent->m_pNextCombinedEvent->m_pNextCombinedEvent);
			/* �s�b�`�x���h�Z���V�e�B�r�e�B */
			/* �R�[�X�`���[�� */
			if (usBankMSB == 0 && (usBankLSB == 0 || usBankLSB == 2)) {
				*p = 0x82;
				*(p + 1) = (unsigned char)(0x80 + usBankLSB);
				memcpy (p + 2, &lTime, 4);
				memcpy (p + 6, &usDummy, 2);
				memcpy (p + 8, &usValue, 2);
				p += 10;
			}
			/* �t�@�C���`���[��(TODO:�l�͕ۗ�:0x0000-0x4000-0x8000) */
			else if (usBankMSB == 0 && usBankLSB == 1) {
				usValue = usValue << 8;
				*p = 0x82;
				*(p + 1) = (unsigned char)(0x80 + usBankLSB);
				memcpy (p + 2, &lTime, 4);
				memcpy (p + 6, &usDummy, 2);
				memcpy (p + 8, &usValue, 2);
				p += 10;
			}
			/* ���̑���RPN�`�F���W */
			else {
				*p = 0x82;
				*(p + 1) = 101;
				memcpy (p + 2, &lTime, 4);
				memcpy (p + 6, &usDummy, 2);
				memcpy (p + 8, &usBankMSB, 2);
				p += 10;
				*p = 0x82;
				*(p + 1) = 101;
				memcpy (p + 2, &lTime, 4);
				memcpy (p + 6, &usDummy, 2);
				memcpy (p + 8, &usBankLSB, 2);
				p += 10;
				*p = 0x82;
				*(p + 1) = 6;
				memcpy (p + 2, &lTime, 4);
				memcpy (p + 6, &usDummy, 2);
				memcpy (p + 8, &usValue, 2);
				p += 10;
			}
			pMIDIEvent = MIDIEvent_GetLastCombinedEvent (pMIDIEvent);
		}
		/* NRPN�`�F���W(CC#99+CC#98+CC#6) */
		else if (MIDIEvent_IsNRPNChange (pMIDIEvent)) {
			long j = 0;
			long lFound = 0;
			long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
			unsigned short usBankMSB = (unsigned short)MIDIEvent_GetBankMSB (pMIDIEvent);
			unsigned short usBankLSB = (unsigned short)MIDIEvent_GetBankLSB (pMIDIEvent);
			unsigned short usDummy = 0x0000;
			unsigned short usValue = (unsigned short)MIDIEvent_GetValue
				(pMIDIEvent->m_pNextCombinedEvent->m_pNextCombinedEvent);
			/* V.Rate / V.Dep / V.Del / HPF.Cutoff / E.G.Delay */
			if (lFound == 0) {
				unsigned char ucBankLSB8C[5] = {8, 9, 10, 36, 100};
				for (j = 0; j < 5; j++) {
					if (usBankMSB == 1 && usBankLSB == ucBankLSB8C[j]) {
						*p = 0x82;
						*(p + 1) = (unsigned char)(0x8C + j); /* 0x8C�`0x90 */
						memcpy (p + 2, &lTime, 4);
						memcpy (p + 6, &usDummy, 2);
						memcpy (p + 8, &usValue, 2);
						p += 10;
						lFound++;
						break;
					}
				}
			}
			/* EQ */
			if (lFound == 0) {
				unsigned char ucBankLSBAF[4] = {48, 49, 52, 53};
				for (j = 0; j < 4; j++) {
					if (usBankMSB == 1 && usBankLSB == ucBankLSBAF[j]) {
						*p = 0x82;
						*(p + 1) = (unsigned char)(0xAF + j); /* 0xAF�`0xB2*/
						memcpy (p + 2, &lTime, 4);
						memcpy (p + 6, &usDummy, 2);
						memcpy (p + 8, &usValue, 2);
						p += 10;
						lFound++;
						break;
					}
				}
			}
			/* Drum params */
			if (lFound == 0) {
				unsigned char ucBankMSBC8[16] = {20, 21, 22, 23, 24, 25, 26, 28, 29, 30, 31, 36, 52, 48, 53, 49};
				for (j = 0; j < 16; j++) {
					if (usBankMSB == ucBankMSBC8[j]) {
						*p = 0x82;
						*(p + 1) = (unsigned char)(0xC8 + j); /* 0xC8�`0xD7 */
						memcpy (p + 2, &lTime, 4);
						memcpy (p + 6, &usBankLSB, 2); /* Drum.Key */
						memcpy (p + 8, &usValue, 2);
						p += 10;
						lFound++;
						break;
					}
				}
			}
			/* ���̑���NRPN�`�F���W */
			if (lFound == 0) {
				*p = 0x82;
				*(p + 1) = 99;
				memcpy (p + 2, &lTime, 4);
				memcpy (p + 6, &usDummy, 2);
				memcpy (p + 8, &usBankMSB, 2);
				p += 10;
				*p = 0x82;
				*(p + 1) = 98;
				memcpy (p + 2, &lTime, 4);
				memcpy (p + 6, &usDummy, 2);
				memcpy (p + 8, &usBankLSB, 2);
				p += 10;
				*p = 0x82;
				*(p + 1) = 6;
				memcpy (p + 2, &lTime, 4);
				memcpy (p + 6, &usDummy, 2);
				memcpy (p + 8, &usValue, 2);
				p += 10;
			}
			pMIDIEvent = MIDIEvent_GetLastCombinedEvent (pMIDIEvent);
		}
		/* �L�[�A�t�^�[�^�b�` */
		else if (MIDIEvent_IsKeyAftertouch (pMIDIEvent)) {
			long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
			unsigned short usDummy = 0x0000;
			unsigned short usValue = (unsigned short)MIDIEvent_GetValue (pMIDIEvent);
			*p = 0x82;
			*(p + 1) = 0x97;
			memcpy (p + 2, &lTime, 4);
			memcpy (p + 6, &usDummy, 2);
			memcpy (p + 8, &usValue, 2);
			p += 10;
		}
		/* �s�b�`�x���h */
		else if (MIDIEvent_IsPitchBend (pMIDIEvent)) {
			long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
			unsigned short usDummy = 0x0000;
			unsigned short usValue = (unsigned short)MIDIEvent_GetValue (pMIDIEvent);
			*p = 0x82;
			*(p + 1) = 0x99;
			memcpy (p + 2, &lTime, 4);
			memcpy (p + 6, &usDummy, 2);
			memcpy (p + 8, &usValue, 2);
			p += 10;
		}
		/* �R���g���[���`�F���W */
		else if (MIDIEvent_IsControlChange (pMIDIEvent)) {
			long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
			unsigned char ucNum = (unsigned char)(CLIP (0, MIDIEvent_GetNumber (pMIDIEvent), 127));
			unsigned short usVal = (unsigned short)(CLIP (0, MIDIEvent_GetValue (pMIDIEvent), 127));
			*p = 0x82;
			*(p + 1) = ucNum;
			memcpy (p + 2, &lTime, 4);
			memset (p + 6, 0x00, 2);
			memcpy (p + 8, &usVal, 2);
			p += 10;
		}
		/* �v���O�����`�F���W */
		else if (MIDIEvent_IsProgramChange (pMIDIEvent)) {
			long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
			unsigned short usVal = (unsigned short)(CLIP (0, MIDIEvent_GetValue (pMIDIEvent), 127));
			*p = 0x83;
			*(p + 1) = 0x00;
			memcpy (p + 2, &lTime, 4);
			memset (p + 6, 0xFF, 2);
			memcpy (p + 8, &usVal, 2);
			p += 10;
		}
		/* �V�X�e���G�N�X�N���[�V�� */
		else if (pMIDIEvent->m_lKind == MIDIEVENT_SYSEXSTART) {
			long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
			long lLen = MIDIEvent_GetLen (pMIDIEvent);
			unsigned char bySysx[256];
			memset (bySysx, 0, sizeof (bySysx));
			MIDIEvent_GetData (pMIDIEvent, bySysx, MIN (lLen, sizeof (bySysx)));
			/* �}�X�^�[�{�����[�� */
			if (bySysx[0] == 0xF0 &&
				bySysx[1] == 0x7F &&
				bySysx[2] == 0x7F &&
				bySysx[3] == 0x04 &&
				bySysx[4] == 0x01 &&
				bySysx[5] == 0x00 &&
				bySysx[7] == 0xF7) {
				unsigned short usVal = bySysx[6];
				*p = 0x82;
				*(p + 1) = 0x83;
				memcpy (p + 2, &lTime, 4);
				memset (p + 6, 0x00, 2);
				memcpy (p + 8, &usVal, 2);
				p += 10;
			}
			/* PEG InitLev / PEG AttackTime / PEG RelLev / PEG RelTime */
			else if (bySysx[0] == 0xF0 &&
				bySysx[1] == 0x43 &&
				bySysx[2] == 0x10 &&
				bySysx[3] == 0x4C &&
				bySysx[4] == 0x08 &&
				bySysx[5] == 0x01 &&
				0x69 <= bySysx[6] && bySysx[6] <= 0x6C &&
				bySysx[8] == 0xF7) {
				unsigned char ucNum = 0x28 + bySysx[6]; /* 0x91�`0x94 */
				unsigned short usVal = bySysx[7];
				*p = 0x82;
				*(p + 1) = ucNum;
				memcpy (p + 2, &lTime, 4);
				memset (p + 6, 0x00, 2);
				memcpy (p + 8, &usVal, 2);
				p += 10;
			}
			/* GM System On */
			else if (bySysx[0] == 0xF0 &&
				bySysx[1] == 0x7E &&
				bySysx[2] == 0x7F &&
				bySysx[3] == 0x09 &&
				bySysx[4] == 0x01 &&
				bySysx[5] == 0xF7) {
				*p = 0x82;
				*(p + 1) = 0xA0;
				memcpy (p + 2, &lTime, 4);
				memset (p + 6, 0x00, 2);
				memset (p + 8, 0x00, 2);
				p += 10;
			}
			/* XG System On */
			else if (bySysx[0] == 0xF0 &&
				bySysx[1] == 0x43 &&
				bySysx[2] == 0x10 &&
				bySysx[3] == 0x4C &&
				bySysx[4] == 0x00 &&
				bySysx[5] == 0x00 &&
				bySysx[6] == 0x7E &&
				bySysx[7] == 0x00 &&
				bySysx[8] == 0xF7) {
				*p = 0x82;
				*(p + 1) = 0xA1;
				memcpy (p + 2, &lTime, 4);
				memset (p + 6, 0x00, 2);
				memset (p + 8, 0x00, 2);
				p += 10;
			}
			/* GS Reset */
			else if (bySysx[0] == 0xF0 &&
				bySysx[1] == 0x41 &&
				bySysx[2] == 0x10 &&
				bySysx[3] == 0x42 &&
				bySysx[4] == 0x12 &&
				bySysx[5] == 0x40 &&
				bySysx[6] == 0x00 &&
				bySysx[7] == 0x7F &&
				bySysx[8] == 0x00 &&
				bySysx[9] == 0x41 &&
				bySysx[10] == 0xF7) {
				*p = 0x82;
				*(p + 1) = 0xA1;
				memcpy (p + 2, &lTime, 4);
				memset (p + 6, 0x00, 2);
				memset (p + 8, 0x00, 2);
				p += 10;
			}
			/* 88 Reset */
			else if (bySysx[0] == 0xF0 &&
				bySysx[1] == 0x41 &&
				bySysx[2] == 0x10 &&
				bySysx[3] == 0x42 &&
				bySysx[4] == 0x12 &&
				bySysx[5] == 0x00 &&
				bySysx[6] == 0x00 &&
				bySysx[7] == 0x7F &&
				0x00 <= bySysx[8] && bySysx[8] <= 0x01 &&
				bySysx[9] == ((128 - (Sum (&bySysx[5], 4) % 128)) & 0x7F) &&
				bySysx[10] == 0xF7) {
				*p = 0x82;
				*(p + 1) = 0xA2;
				memcpy (p + 2, &lTime, 4);
				memset (p + 6, 0x00, 2);
				memset (p + 8, 0x00, 2);
				*(p + 8) = bySysx[8];
				p += 10;
			}
			/* Dry Level */
			else if (bySysx[0] == 0xF0 &&
				bySysx[1] == 0x43 &&
				bySysx[2] == 0x10 &&
				bySysx[3] == 0x4C &&
				bySysx[4] == 0x08 &&
				bySysx[5] == 0x03 &&
				bySysx[6] == 0x11 &&
				bySysx[8] == 0xF7) {
				unsigned short usVal = bySysx[7];
				*p = 0x82;
				*(p + 1) = 0xA5;
				memcpy (p + 2, &lTime, 4);
				memset (p + 6, 0x00, 2);
				memcpy (p + 8, &usVal, 2);
				p += 10;
			}
			/* MW params */
			else if (bySysx[0] == 0xF0 &&
				bySysx[1] == 0x43 &&
				bySysx[2] == 0x10 &&
				bySysx[3] == 0x4C &&
				bySysx[4] == 0x08 &&
				bySysx[5] == 0x02 &&
				0x1D <= bySysx[6] && bySysx[6] <= 0x22 &&
				bySysx[8] == 0xF7) {
				unsigned char ucNum = bySysx[6] - 0x1D + 0xA6; /* 0xA6�`0xAB */
				unsigned short usVal = bySysx[7];
				*p = 0x82;
				*(p + 1) = ucNum;
				memcpy (p + 2, &lTime, 4);
				memset (p + 6, 0x00, 2);
				memcpy (p + 8, &usVal, 2);
				p += 10;
			}
			/* GS: Reverb / Chorus / Delay Macro  */
			else if (bySysx[0] == 0xF0 &&
				bySysx[1] == 0x41 &&
				bySysx[2] == 0x10 &&
				bySysx[3] == 0x42 &&
				bySysx[4] == 0x12 &&
				bySysx[5] == 0x40 &&
				bySysx[6] == 0x01 &&
				(bySysx[7] == 0x30 || bySysx[7] == 0x38 || bySysx[7] == 0x50) &&
				0x00 <= bySysx[8] && bySysx[8] <= 0x07 &&
				bySysx[9] == ((128 - (Sum (&bySysx[5], 4) % 128)) & 0x7F) &&
				bySysx[10] == 0xF7) {
				*p = 0x82;
				*(p + 1) = bySysx[7] == 0x30 ? 0xAC : (bySysx[7] == 0x38 ? 0xAD : 0xAE);
				memcpy (p + 2, &lTime, 4);
				memset (p + 6, 0x00, 2);
				memset (p + 8, 0x00, 2);
				*(p + 8) = bySysx[8];
				p += 10;
			}
			/* GS: EQ */
			else if (bySysx[0] == 0xF0 &&
				bySysx[1] == 0x41 &&
				bySysx[2] == 0x10 &&
				bySysx[3] == 0x42 &&
				bySysx[4] == 0x12 &&
				bySysx[5] == 0x40 &&
				bySysx[6] == 0x02 &&
				0x00 <= bySysx[7] && bySysx[7] <= 0x03 &&
				0x00 <= bySysx[8] && bySysx[8] <= 0x7F &&
				bySysx[9] == ((128 - (Sum (&bySysx[5], 4) % 128)) & 0x7F) &&
				bySysx[10] == 0xF7) {
				*p = 0x82;
				*(p + 1) = 0xAF + bySysx[7];
				memcpy (p + 2, &lTime, 4);
				memset (p + 6, 0x00, 2);
				memset (p + 8, 0x00, 2);
				*(p + 8) = bySysx[8];
				p += 10;
			}
			/* DX:AssignPart */
			else if (bySysx[0] == 0xF0 &&
				bySysx[1] == 0x43 &&
				bySysx[2] == 0x10 &&
				bySysx[3] == 0x4C &&
				bySysx[4] == 0x70 &&
				bySysx[5] == 0x02 &&
				bySysx[6] == 0x00 &&
				bySysx[7] == 0x02 &&
				bySysx[8] == 0xF7) {
				*p = 0x82;
				*(p + 1) = 0xB3;
				memcpy (p + 2, &lTime, 4);
				memset (p + 6, 0x00, 2);
				memset (p + 8, 0x00, 2);
				p += 10;
			}
			/* GS:Assign Mode */
			else if (bySysx[0] == 0xF0 &&
				bySysx[1] == 0x41 &&
				bySysx[2] == 0x10 &&
				bySysx[3] == 0x42 &&
				bySysx[4] == 0x12 &&
				bySysx[5] == 0x40 &&
				bySysx[6] == 0x17 &&
				bySysx[7] == 0x14 &&
				0x00 <= bySysx[8] && bySysx[8] <= 0x7F &&
				bySysx[9] == ((128 - (Sum (&bySysx[5], 4) % 128)) & 0x7F) &&
				bySysx[10] == 0xF7) {
				*p = 0x82;
				*(p + 1) = 0xB3;
				memcpy (p + 2, &lTime, 4);
				memset (p + 6, 0x00, 2);
				memset (p + 8, 0x00, 2);
				*(p + 8) = bySysx[8];
				p += 10;
			}
			/* GS: Use For Norm/Drum1/Drum2 */
			else if (bySysx[0] == 0xF0 &&
				bySysx[1] == 0x41 &&
				bySysx[2] == 0x10 &&
				bySysx[3] == 0x42 &&
				bySysx[4] == 0x12 &&
				bySysx[5] == 0x40 &&
				bySysx[6] == 0x17 &&
				bySysx[7] == 0x15 &&
				0x00 <= bySysx[8] && bySysx[8] <= 0x02 &&
				bySysx[9] == ((128 - (Sum (&bySysx[5], 4) % 128)) & 0x7F) &&
				bySysx[10] == 0xF7) {
				*p = 0x82;
				*(p + 1) = 0xB4;
				memcpy (p + 2, &lTime, 4);
				memset (p + 6, 0x00, 2);
				memset (p + 8, 0x00, 2);
				*(p + 8) = bySysx[8];
				p += 10;
			}
			/* GS: EQ SW Off/On */
			else if (bySysx[0] == 0xF0 &&
				bySysx[1] == 0x41 &&
				bySysx[2] == 0x10 &&
				bySysx[3] == 0x42 &&
				bySysx[4] == 0x12 &&
				bySysx[5] == 0x40 &&
				bySysx[6] == 0x47 &&
				bySysx[7] == 0x20 &&
				0x00 <= bySysx[8] && bySysx[8] <= 0x01 &&
				bySysx[9] == ((128 - (Sum (&bySysx[5], 4) % 128)) & 0x7F) &&
				bySysx[10] == 0xF7) {
				*p = 0x82;
				*(p + 1) = 0xB5;
				memcpy (p + 2, &lTime, 4);
				memset (p + 6, 0x00, 2);
				memset (p + 8, 0x00, 2);
				*(p + 8) = bySysx[8];
				p += 10;
			}
			/* ���̑��̃V�X�e���G�N�X�N���[�V�� */
			else {
				*p = 0xC0;
				*(p + 1) = 0x00;
				memcpy (p + 2, &lTime, 4);
				memset (p + 6, 0x00, 2);
				memset (p + 8, 0x00, 2);
				memcpy (p + 10, &lLen, 4);
				memcpy (p + 14, bySysx, lLen);
				p += (14 + lLen);
			}
		}
		/* �e�L�X�g */
		else if (pMIDIEvent->m_lKind == MIDIEVENT_TEXTEVENT) {
			long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
			long lLen = 0;
			memset (szTempBuf, 0, sizeof (szTempBuf));
			lLen = MIDIEvent_GetData (pMIDIEvent, szTempBuf, sizeof (szTempBuf) - 1);
			lLen += 1;
			*p = 0xC1;
			*(p + 1) = 0x00;
			memcpy (p + 2, &lTime, 4);
			memset (p + 6, 0x00, 2);
			memset (p + 8, 0x00, 2);
			memcpy (p + 10, &lLen, 4);
			memcpy (p + 14, szTempBuf, lLen);
			p += (14 + lLen);
		}
		/* �̎� */
		else if (pMIDIEvent->m_lKind == MIDIEVENT_LYRIC) {
			long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
			long lLen = 0;
			memset (szTempBuf, 0, sizeof (szTempBuf));
			lLen = MIDIEvent_GetData (pMIDIEvent, szTempBuf, sizeof (szTempBuf) - 1);
			lLen += 1;
			*p = 0xC2;
			*(p + 1) = 0x00;
			memcpy (p + 2, &lTime, 4);
			memset (p + 6, 0x00, 2);
			memset (p + 8, 0x00, 2);
			memcpy (p + 10, &lLen, 4);
			memcpy (p + 14, szTempBuf, lLen);
			p += (14 + lLen);
		}
		/* �G���h�I�u�g���b�N */
		else if (MIDIEvent_IsEndofTrack (pMIDIEvent)) {
			long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
			*p = 0xFF;
			*(p + 1) = 0x00;
			memcpy (p + 2, &lTime, 4);
			memset (p + 6, 0x00, 2);
			memset (p + 8, 0x00, 2);
			p += 10;
		}
	}
	return (p - pTrackData);
}

/* MIDITrack��Cherry�`���Ń������u���b�N��ɕۑ�(���q����)(����J) */
long __stdcall MIDITrack_SaveAsCherry3 (MIDITrack* pMIDITrack, unsigned char* pTrackData, long lTrackLen) {
	MIDIEvent* pMIDIEvent = NULL;
	unsigned char* p = pTrackData;
	long lOldTime = -1;
	assert (pMIDITrack);
	assert (pMIDITrack->m_lTempIndex == 0);
	assert (pTrackData);
	assert (0 <= lTrackLen && lTrackLen <= 0x7FFFFFFF);

	/* ���q�L���E�����L���C�x���g�̂ݏ������� */
	forEachEvent (pMIDITrack, pMIDIEvent) {
		unsigned char byBuf[64];
		strcpy ((char*)byBuf, "Setup");
		/* �}�[�J�[ */
		if (MIDIEvent_IsMarker (pMIDIEvent)) {
			long lsf = 0;
			long lmi = 0;
			long lMeasure, lBeat, lTick;
			long lnn, ldd, lcc, lbb;
			long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
			unsigned short usMeasure = 0;
			unsigned char ucnn = 4;
			unsigned char ucdd = 2;
			unsigned char ucsf = 0;
			unsigned char ucmi = 0;
			memset (byBuf, 0, sizeof (byBuf));
			MIDIEvent_GetData (pMIDIEvent, byBuf, sizeof (byBuf));
			MIDITrack_BreakTime (pMIDITrack, lTime, &lMeasure, &lBeat, &lTick);
			MIDITrack_MakeTime (pMIDITrack, lMeasure, 0, 0, &lTime);
			MIDITrack_FindKeySignature (pMIDITrack, lTime, &lsf, &lmi); 
			MIDITrack_FindTimeSignature (pMIDITrack, lTime, &lnn, &ldd, &lcc, &lbb);
			usMeasure = (unsigned short)(CLIP (0, lMeasure, 65535));
			ucnn = (unsigned char)(CLIP (0, lnn, 255));
			ucdd = (unsigned char)(CLIP (0, 1 << ldd, 255));
			ucsf = (unsigned char)(0 <= lsf && lsf <= 8 ? lsf : 8 - lsf);
			ucmi = (unsigned char)(CLIP (0, lmi, 1));
			if (lOldTime == lTime) {
				p -= 80;
			}
			memset (p, 0, 80);
			memcpy (p, &usMeasure, 2);
			*(p + 2) = ucnn;
			*(p + 3) = ucdd;
			*(p + 4) = ucsf;
			*(p + 5) = ucmi;
			strncpy ((char*)(p + 16), (char*)byBuf, 63);
			p += 80;
			lOldTime = lTime;
		}
		/* ���q�L�� */
		else if (MIDIEvent_IsTimeSignature (pMIDIEvent)) {
			long lsf = 0;
			long lmi = 0;
			long lMeasure, lBeat, lTick;
			long lnn, ldd, lcc, lbb;
			long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
			unsigned short usMeasure = 0;
			unsigned char ucnn = 4;
			unsigned char ucdd = 2;
			unsigned char ucsf = 0;
			unsigned char ucmi = 0;
			MIDITrack_BreakTime (pMIDITrack, lTime, &lMeasure, &lBeat, &lTick);
			MIDITrack_MakeTime (pMIDITrack, lMeasure, 0, 0, &lTime);
			MIDITrack_FindKeySignature (pMIDITrack, lTime, &lsf, &lmi); 
			MIDIEvent_GetTimeSignature (pMIDIEvent, &lnn, &ldd, &lcc, &lbb);
			usMeasure = (unsigned short)(CLIP (0, lMeasure, 65535));
			ucnn = (unsigned char)(CLIP (0, lnn, 255));
			ucdd = (unsigned char)(CLIP (0, 1 << ldd, 255));
			ucsf = (unsigned char)(0 <= lsf && lsf <= 8 ? lsf : 8 - lsf);
			ucmi = (unsigned char)(CLIP (0, lmi, 1));
			if (lOldTime == lTime) {
				p -= 80;
			}
			memset (p, 0, 80);
			memcpy (p, &usMeasure, 2);
			*(p + 2) = ucnn;
			*(p + 3) = ucdd;
			*(p + 4) = ucsf;
			*(p + 5) = ucmi;
			strncpy ((char*)(p + 16), (char*)byBuf, 63);
			p += 80;
			lOldTime = lTime;
		}
		/* �����L�� */
		else if (MIDIEvent_IsKeySignature (pMIDIEvent)) {
			long lsf = 0;
			long lmi = 0;
			long lMeasure, lBeat, lTick;
			long lnn, ldd, lcc, lbb;
			long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
			unsigned short usMeasure = 0;
			unsigned char ucnn = 4;
			unsigned char ucdd = 2;
			unsigned char ucsf = 0;
			unsigned char ucmi = 0;
			MIDITrack_BreakTime (pMIDITrack, lTime, &lMeasure, &lBeat, &lTick);
			MIDITrack_MakeTime (pMIDITrack, lMeasure, 0, 0, &lTime);
			MIDIEvent_GetKeySignature (pMIDIEvent, &lsf, &lmi); 
			MIDITrack_FindTimeSignature (pMIDITrack, lTime, &lnn, &ldd, &lcc, &lbb);
			usMeasure = (unsigned short)(CLIP (0, lMeasure, 65535));
			ucnn = (unsigned char)(CLIP (0, lnn, 255));
			ucdd = (unsigned char)(CLIP (0, 1 << ldd, 255));
			ucsf = (unsigned char)(0 <= lsf && lsf <= 8 ? lsf : 8 - lsf);
			ucmi = (unsigned char)(CLIP (0, lmi, 1));
			if (lOldTime == lTime) {
				p -= 80;
			}
			memset (p, 0, 80);
			memcpy (p, &usMeasure, 2);
			*(p + 2) = ucnn;
			*(p + 3) = ucdd;
			*(p + 4) = ucsf;
			*(p + 5) = ucmi;
			strncpy ((char*)(p + 16), (char*)byBuf, 63);
			p += 80;
			lOldTime = lTime;
		}
	}
	return (p - pTrackData);
}




/* MIDITrack��Cherry�`���Ń������u���b�N��ɕۑ�(�w��MIDI�`�����l���C�x���g�̂�)(����J) */
long __stdcall MIDITrack_SaveAsCherryChannel 
	(MIDITrack* pMIDITrack, long lChannel, unsigned char* pTrackData, long lTrackLen) {
	MIDIEvent* pMIDIEvent = NULL;
	unsigned char* p = pTrackData;
	unsigned char szTempBuf[256];
	memset (szTempBuf, 0, sizeof (szTempBuf));

	assert (pMIDITrack);
	assert (pTrackData);
	assert (pMIDITrack->m_lTempIndex == 0);
	assert (0 <= lChannel && lChannel <= 15);
		
	/* �\��̈�(12byte) */
	memset (p, 0x00, 12);
	p += 12;
	
	/* �������f�[�^(48byte) */
	memset (p, 0xFF, 48);
	*(p + 0) = (unsigned char)(CLIP (0, pMIDITrack->m_lViewMode, 2));
	*(p + 1) = (unsigned char)(CLIP (0, pMIDITrack->m_lOutputPort, 3));
	*(p + 2) = (unsigned char)(CLIP (0, lChannel, 15));
	*(p + 3) = (unsigned char)(CLIP (-127, pMIDITrack->m_lKeyPlus, 127));
	*(p + 4) = (unsigned char)(CLIP (-127, pMIDITrack->m_lTimePlus, 127));
	*(p + 5) = (unsigned char)(CLIP (-127, pMIDITrack->m_lVelocityPlus, 127));
	if (pMIDITrack->m_lReserved1 != 0x00000000) {
		*(p + 16) = (unsigned char)((pMIDITrack->m_lReserved1 >> 0) & 0xFF);
		*(p + 17) = (unsigned char)((pMIDITrack->m_lReserved1 >> 8) & 0xFF);
		*(p + 18) = (unsigned char)((pMIDITrack->m_lReserved1 >> 16) & 0xFF);
		*(p + 19) = (unsigned char)((pMIDITrack->m_lReserved1 >> 24) & 0xFF);
	}
	p += 48;

	/* �g���b�N��(64byte) */
	memset (p, 0x00, 64);
	MIDITrack_GetName (pMIDITrack, (char*)p, 63);
	p += 64;

	/* �m�[�g�I���ECC#�EPC#�ESysx�E�e�L�X�g�E�̎��C�x���g�̂ݏ������� */
	forEachEvent (pMIDITrack, pMIDIEvent) {
		/* �m�[�g�I��(�m�[�g�I�t�Ɍ������Ă�����̂̂�) */
		if (MIDIEvent_IsNoteOn (pMIDIEvent) && pMIDIEvent->m_pNextCombinedEvent) {
			if (MIDIEvent_GetChannel (pMIDIEvent) == lChannel) {
				long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
				unsigned char ucKey = (unsigned char)(CLIP (0, MIDIEvent_GetKey (pMIDIEvent), 127));
				unsigned short usDur = (unsigned short)(CLIP (0, MIDIEvent_GetDuration (pMIDIEvent), 65535));
				unsigned short usVel = (unsigned short)(CLIP (0, MIDIEvent_GetVelocity (pMIDIEvent), 127));
				*p = ucKey;
				*(p + 1) = 0;
				memcpy (p + 2, &lTime, 4);
				memcpy (p + 6, &usDur, 2);
				memcpy (p + 8, &usVel, 2);
				p += 10;
			}
		}
		/* �R���g���[���`�F���W */
		else if (MIDIEvent_IsControlChange (pMIDIEvent)) {
			if (MIDIEvent_GetChannel (pMIDIEvent) == lChannel) {
				long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
				unsigned char ucNum = (unsigned char)(CLIP (0, MIDIEvent_GetNumber (pMIDIEvent), 127));
				unsigned short usVal = (unsigned short)(CLIP (0, MIDIEvent_GetValue (pMIDIEvent), 127));
				*p = 0x82;
				*(p + 1) = ucNum;
				memcpy (p + 2, &lTime, 4);
				memset (p + 6, 0x00, 2);
				memcpy (p + 8, &usVal, 2);
				p += 10;
			}
		}
		/* �v���O�����`�F���W */
		else if (MIDIEvent_IsProgramChange (pMIDIEvent)) {
			if (MIDIEvent_GetChannel (pMIDIEvent) == lChannel) {
				long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
				unsigned short usVal = (unsigned short)(CLIP (0, MIDIEvent_GetValue (pMIDIEvent), 127));
				*p = 0x83;
				*(p + 1) = 0x00;
				memcpy (p + 2, &lTime, 4);
				memset (p + 6, 0xFF, 2);
				memcpy (p + 8, &usVal, 2);
				p += 10;
			}
		}
		/* �G���h�I�u�g���b�N */
		else if (MIDIEvent_IsEndofTrack (pMIDIEvent)) {
			long lTime = CLIP (0, MIDIEvent_GetTime (pMIDIEvent), 0x7FFFFFFF);
			*p = 0xFF;
			*(p + 1) = 0x00;
			memcpy (p + 2, &lTime, 4);
			memset (p + 6, 0x00, 2);
			memset (p + 8, 0x00, 2);
			p += 10;
		}
	}
	return (p - pTrackData);
}

/* MIDI�g���b�N��Cherry�Ō����Ƃ���̃V�X�e���Z�b�g�A�b�v�g���b�N���ǂ������ׂ� */
long __stdcall MIDITrack_IsSystemSetupTrack (MIDITrack* pMIDITrack) {
	/* �V�X�e���R���_�N�^�[�g���b�N��MIDI�g���b�N�̍ŏ��̃g���b�N�̂����A*/
	/* {�e���|�ESMPTE�I�t�Z�b�g�E���q�L���E�����L���E�}�[�J�[}�ȊO���ڂ���B */
	/* �V�X�e���Z�b�g�A�b�v�g���b�N�͊�{�I�ɂ�SysEx��p�g���b�N�ł���A���ɂ� */
	/* �g���b�N��(�ʏ�"System Setup")�A�R�����g�p�e�L�X�g�A�G���h�I�u�g���b�N������B */
	MIDIEvent* pMIDIEvent = NULL;
	forEachEvent (pMIDITrack, pMIDIEvent) {
		long lKind = MIDIEvent_GetKind (pMIDIEvent);
		/* MIDI�`�����l���C�x���g�̋֎~ */
		if (0x80 <= lKind && lKind <= 0xEF) {
			return 0;
		}
		/* �e���|�ESMPTE�I�t�Z�b�g�E���q�L���E�����L���̋֎~ */
		if (0x50 <= lKind && lKind <= 0x5F) {
			return 0;	
		}
		/* �V�[�P���X�ԍ��E���쌠�E�}�[�J�[�E�L���[�|�C���g�̋֎~ */
		if (lKind == MIDIEVENT_SEQUENCENUMBER || lKind == MIDIEVENT_COPYRIGHTNOTICE ||
			lKind == MIDIEVENT_MARKER || lKind == MIDIEVENT_CUEPOINT) {
			return 0;
		}
	}
	return 1;
}

/* MIDI�f�[�^��Cherry�t�@�C��(*.chy)�ɕۑ� */
long __stdcall MIDIData_SaveAsCherry (MIDIData* pMIDIData, const char* pszFileName) {

	MIDITrack* pMIDITrack = NULL;
	FILE* pFile = NULL;
	long lGuessTrackLen = 0;
	long lTrackLen = 0;
	long lTrackLen4 = 0;
	long lTrackIndex = 0;
	unsigned short usFormat = (unsigned short)MIDIData_GetFormat (pMIDIData);
	unsigned short usTimeMode = (unsigned short)MIDIData_GetTimeMode (pMIDIData);
	unsigned short usTimeResolution = (unsigned short)MIDIData_GetTimeResolution (pMIDIData);
	unsigned short usNumTrack = (usFormat == 0 ? 18 : (unsigned short)MIDIData_CountTrack (pMIDIData));
	unsigned char* pTrackData = NULL;
	unsigned char byBuf[256];
	char szTempTitle[256];
	char szTempCopyright[256];
	long lHasSystemSetupTrack = 0;
	long lTempExtendedDataAddress[64];
	long lTempTrackAddress[65536];
	memset (szTempTitle, 0, sizeof (szTempTitle));
	memset (szTempCopyright, 0, sizeof (szTempCopyright));
	memset (lTempExtendedDataAddress, 0, sizeof (long) * 64);
	memset (lTempTrackAddress, 0, sizeof (long) * 65536);

	/* �ŏI�I�ȕۑ��g���b�N���̌v�Z */
	if (usFormat == 0) {
		usNumTrack = 18;
	}
	else {
		usNumTrack = (unsigned short)CLIP (0, MIDIData_CountTrack (pMIDIData), 65535);
		if (usNumTrack >= 2) {
			MIDITrack* pSecondTrack = pMIDIData->m_pFirstTrack->m_pNextTrack;
			/* �V�X�e���Z�b�g�A�b�v��p�g���b�N���Ȃ��ꍇ�A */
			/* �V�X�e���Z�b�g�A�b�v��p�g���b�N��ǉ��ۑ�����̂�+1 */
			if (MIDITrack_IsSystemSetupTrack (pSecondTrack) == 0) {
				usNumTrack++;
			}
			/* �V�X�e���Z�b�g�A�b�v��p�g���b�N������ꍇ */
			else {
				lHasSystemSetupTrack = 1;
			}
		}
		/* �V�X�e���Z�b�g�A�b�v��p�g���b�N���Ȃ��ꍇ�A */
		/* �V�X�e���Z�b�g�A�b�v��p�g���b�N��ǉ��ۑ�����̂�+1 */
		else {
			usNumTrack++;
		}
	}

	/* �t�@�C���I�[�v�� */
	pFile = fopen (pszFileName, "wb");
	if (pFile == NULL) {
		return 0;
	}

	/* �w�b�_�[(256byte) */
	memset (&byBuf[0], 0, sizeof (byBuf));
	strcpy ((char*)(&byBuf[0]), "CHRY0101");
	memcpy (&byBuf[8], &usTimeResolution, 2);
	memcpy (&byBuf[10], &usNumTrack, 2);
	if (fwrite (byBuf, 1, 256, pFile) < 256) {
		fclose (pFile);
		return 0;
	}
	/* �Ȗ��ƒ��쌠(128+128=256byte) */
	memset (&byBuf[0], 0, sizeof (byBuf));
	MIDIData_GetTitle (pMIDIData, (char*)(&byBuf[0]), 127);
	MIDIData_GetCopyright (pMIDIData, (char*)(&byBuf[128]), 127);
	if (fwrite (byBuf, 1, 256, pFile) < 256) {
		fclose (pFile);
		return 0;
	}
	/* �g���f�[�^�̐擪�A�h���X(4*64=256byte) */
	memset (lTempExtendedDataAddress, 0, sizeof (long) * 64);
	if (fwrite (lTempExtendedDataAddress, sizeof (long), 64, pFile) < 64) {
		fclose (pFile);
		return 0;
	}
	/* �g���b�N�̐擪�A�h���X(4*usNumTrack byte) */
	memset (lTempTrackAddress, 0, sizeof (long) * 65536);
	if (fwrite (lTempTrackAddress, sizeof (long), usNumTrack, pFile) < usNumTrack) {
		fclose (pFile);
		return 0;
	}

	/* �t�H�[�}�b�g0�̏ꍇ */
	if (usFormat == 0) {
		long i;
		MIDITrack* pMIDITrack = MIDIData_GetFirstTrack (pMIDIData);
		/* �e���|��p�g���b�N */
		lTempTrackAddress[lTrackIndex] = ftell (pFile);
		lGuessTrackLen = MIDITrack_GuessTrackDataLenAsCherry (pMIDITrack);
		pTrackData = malloc (lGuessTrackLen);
		if (pTrackData == NULL) {
			fclose (pFile);
			return 0;
		}
		memset (pTrackData, 0, lGuessTrackLen);
		lTrackLen = MIDITrack_SaveAsCherry0 (pMIDITrack, pTrackData, lGuessTrackLen);
		assert (124 <= lTrackLen && lTrackLen < lGuessTrackLen);
		lTrackLen4 = lTrackLen + 4;
		/* �g���b�N����(4byte) */
		if (fwrite (&lTrackLen4, sizeof (long), 1, pFile) < 1) {
			free (pTrackData);
			fclose (pFile);
			return 0;
		}
		/* �g���b�N�f�[�^(lTrackLenbyte) */
		if (fwrite (pTrackData, 1, lTrackLen, pFile) < (unsigned long)lTrackLen) {
			free (pTrackData);
			fclose (pFile);
			return 0;
		}
		lTrackIndex++;
		/* �V�X�e���Z�b�g�A�b�v�p�g���b�N */
		lTempTrackAddress[lTrackIndex] = ftell (pFile);
		memset (pTrackData, 0, lGuessTrackLen);
		lTrackLen = MIDITrack_SaveAsCherry1 (pMIDITrack, pTrackData, lGuessTrackLen);
		assert (124 <= lTrackLen && lTrackLen < lGuessTrackLen);
		lTrackLen4 = lTrackLen + 4;
		/* �g���b�N����(4byte) */
		if (fwrite (&lTrackLen4, sizeof (long), 1, pFile) < 1) {
			free (pTrackData);
			fclose (pFile);
			return 0;
		}
		/* �g���b�N�f�[�^(lTrackLenbyte) */
		if (fwrite (pTrackData, 1, lTrackLen, pFile) < (unsigned long)lTrackLen) {
			free (pTrackData);
			fclose (pFile);
			return 0;
		}
		lTrackIndex++;
		/* �`�����l��1�`16�p�g���b�N */		
		for (i = 0; i < 16; i++) {
			lTempTrackAddress[lTrackIndex] = ftell (pFile);
			lGuessTrackLen = MIDITrack_GuessTrackDataLenAsCherry (pMIDITrack);
			pTrackData = malloc (lGuessTrackLen);
			if (pTrackData == NULL) {
				fclose (pFile);
				return 0;
			}
			memset (pTrackData, 0, lGuessTrackLen);
			lTrackLen = MIDITrack_SaveAsCherryChannel (pMIDITrack, i, pTrackData, lGuessTrackLen);
			assert (124 <= lTrackLen && lTrackLen < lGuessTrackLen);
			lTrackLen4 = lTrackLen + 4;
			/* �g���b�N����(4byte) */
			if (fwrite (&lTrackLen4, sizeof (long), 1, pFile) < 1) {
				free (pTrackData);
				fclose (pFile);
				return 0;
			}
			/* �g���b�N�f�[�^(lTrackLenbyte) */
			if (fwrite (pTrackData, 1, lTrackLen, pFile) < (unsigned long)lTrackLen) {
				free (pTrackData);
				fclose (pFile);
				return 0;
			}
			pTrackData = NULL;
			free (pTrackData);
			lTrackIndex++;
		}
	}
	
	/* �t�H�[�}�b�g1,2�̏ꍇ */
	else {
		/* �e�g���b�N�̕ۑ� */
		lTrackIndex = 0;
		forEachTrack (pMIDIData, pMIDITrack) {
			/* �g���b�N0 */
			if (pMIDITrack->m_lTempIndex == 0) {
				/* �g���b�N0�̃e���|���̂ݕۑ� */
				lTempTrackAddress[lTrackIndex] = ftell (pFile);
				lGuessTrackLen = MIDITrack_GuessTrackDataLenAsCherry (pMIDITrack);
				pTrackData = malloc (lGuessTrackLen);
				if (pTrackData == NULL) {
					fclose (pFile);
					return 0;
				}
				memset (pTrackData, 0, lGuessTrackLen);
				lTrackLen = MIDITrack_SaveAsCherry0 (pMIDITrack, pTrackData, lGuessTrackLen);
				assert (124 <= lTrackLen && lTrackLen < lGuessTrackLen);
				lTrackLen4 = lTrackLen + 4;
				/* �g���b�N����(4byte) */
				if (fwrite (&lTrackLen4, sizeof (long), 1, pFile) < 1) {
					free (pTrackData);
					fclose (pFile);
					return 0;
				}
				/* �g���b�N�f�[�^(lTrackLenbyte) */
				if (fwrite (pTrackData, 1, lTrackLen, pFile) < (unsigned long)lTrackLen) {
					free (pTrackData);
					fclose (pFile);
					return 0;
				}
				lTrackIndex++;
				/* �V�X�e���Z�b�g�A�b�v��p�g���b�N���Ȃ��ꍇ */
				if (lHasSystemSetupTrack == 0) {
					/* �g���b�N0����V�X�e���Z�b�g�A�b�v�𐶐����ǉ��g���b�N�Ƃ��ĕۑ� */
					lTempTrackAddress[lTrackIndex] = ftell (pFile);
					lGuessTrackLen = MIDITrack_GuessTrackDataLenAsCherry (pMIDITrack);
					pTrackData = malloc (lGuessTrackLen);
					if (pTrackData == NULL) {
						fclose (pFile);
						return 0;
					}
					memset (pTrackData, 0, lGuessTrackLen);
					lTrackLen = MIDITrack_SaveAsCherry1 (pMIDITrack, pTrackData, lGuessTrackLen);
					assert (124 <= lTrackLen && lTrackLen < lGuessTrackLen);
					lTrackLen4 = lTrackLen + 4;
					/* �g���b�N����(4byte) */
					if (fwrite (&lTrackLen4, sizeof (long), 1, pFile) < 1) {
						free (pTrackData);
						fclose (pFile);
						return 0;
					}
					/* �g���b�N�f�[�^(lTrackLenbyte) */
					if (fwrite (pTrackData, 1, lTrackLen, pFile) < (unsigned long)lTrackLen) {
						free (pTrackData);
						fclose (pFile);
						return 0;
					}
					free (pTrackData);
					pTrackData = NULL;
					lTrackIndex++;
				}
			}
			/* �g���b�N1 */
			else if (pMIDITrack->m_lTempIndex == 1) {
				/* �V�X�e���Z�b�g�A�b�v��p�g���b�N�ł���ꍇ */
				if (lHasSystemSetupTrack == 1) {
					lTempTrackAddress[lTrackIndex] = ftell (pFile);
					lGuessTrackLen = MIDITrack_GuessTrackDataLenAsCherry (pMIDITrack);
					pTrackData = malloc (lGuessTrackLen);
					if (pTrackData == NULL) {
						fclose (pFile);
						return 0;
					}
					memset (pTrackData, 0, lGuessTrackLen);
					lTrackLen = MIDITrack_SaveAsCherry1 (pMIDITrack, pTrackData, lGuessTrackLen);
					assert (124 <= lTrackLen && lTrackLen < lGuessTrackLen);
					lTrackLen4 = lTrackLen + 4;
					/* �g���b�N����(4byte) */
					if (fwrite (&lTrackLen4, sizeof (long), 1, pFile) < 1) {
						free (pTrackData);
						fclose (pFile);
						return 0;
					}
					/* �g���b�N�f�[�^(lTrackLenbyte) */
					if (fwrite (pTrackData, 1, lTrackLen, pFile) < (unsigned long)lTrackLen) {
						free (pTrackData);
						fclose (pFile);
						return 0;
					}
					free (pTrackData);
					pTrackData = NULL;
					lTrackIndex++;
				}
				/* �ʏ�g���b�N�̏ꍇ */
				else {
					lTempTrackAddress[lTrackIndex] = ftell (pFile);
					lGuessTrackLen = MIDITrack_GuessTrackDataLenAsCherry (pMIDITrack);
					pTrackData = malloc (lGuessTrackLen);
					if (pTrackData == NULL) {
						fclose (pFile);
						return 0;
					}
					memset (pTrackData, 0, lGuessTrackLen);
					lTrackLen = MIDITrack_SaveAsCherry2 (pMIDITrack, pTrackData, lGuessTrackLen);
					assert (124 <= lTrackLen && lTrackLen < lGuessTrackLen);
					lTrackLen4 = lTrackLen + 4;
					/* �g���b�N����(4byte) */
					if (fwrite (&lTrackLen4, sizeof (long), 1, pFile) < 1) {
						free (pTrackData);
						fclose (pFile);
						return 0;
					}
					/* �g���b�N�f�[�^(lTrackLenbyte) */
					if (fwrite (pTrackData, 1, lTrackLen, pFile) < (unsigned long)lTrackLen) {
						free (pTrackData);
						fclose (pFile);
						return 0;
					}
					free (pTrackData);
					pTrackData = NULL;
					lTrackIndex++;
				}
			}
			/* �g���b�N2�ȍ~ */
			else {
				/* �ʏ�g���b�N */
				lTempTrackAddress[lTrackIndex] = ftell (pFile);
				lGuessTrackLen = MIDITrack_GuessTrackDataLenAsCherry (pMIDITrack);
				pTrackData = malloc (lGuessTrackLen);
				if (pTrackData == NULL) {
					fclose (pFile);
					return 0;
				}
				memset (pTrackData, 0, lGuessTrackLen);
				lTrackLen = MIDITrack_SaveAsCherry2 (pMIDITrack, pTrackData, lGuessTrackLen);
				assert (124 <= lTrackLen && lTrackLen < lGuessTrackLen);
				lTrackLen4 = lTrackLen + 4;
				/* �g���b�N����(4byte) */
				if (fwrite (&lTrackLen4, sizeof (long), 1, pFile) < 1) {
					free (pTrackData);
					fclose (pFile);
					return 0;
				}
				/* �g���b�N�f�[�^(lTrackLenbyte) */
				if (fwrite (pTrackData, 1, lTrackLen, pFile) < (unsigned long)lTrackLen) {
					free (pTrackData);
					fclose (pFile);
					return 0;
				}
				free (pTrackData);
				pTrackData = NULL;
				lTrackIndex++;
			}
		}
	}

	/* �g���f�[�^ */
	/* ���q���� */
	if (pMIDIData->m_pFirstTrack) {
		pMIDITrack = pMIDIData->m_pFirstTrack;
		lTempExtendedDataAddress[0] = ftell (pFile);
		lGuessTrackLen = MIDITrack_GuessTrackDataLenAsCherry (pMIDITrack);
		pTrackData = malloc (lGuessTrackLen);
		if (pTrackData == NULL) {
			fclose (pFile);
			return 0;
		}
		memset (pTrackData, 0, lGuessTrackLen);
		lTrackLen = MIDITrack_SaveAsCherry3 (pMIDITrack, pTrackData, lGuessTrackLen);
		assert (0 <= lTrackLen && lTrackLen < lGuessTrackLen);
		/* ���q�����f�[�^�̒���(4byte) */
		if (fwrite (&lTrackLen, sizeof (long), 1, pFile) < 1) {
			free (pTrackData);
			fclose (pFile);
			return 0;
		}
		/* ���q�����f�[�^(lTrackLenbyte) */
		if (fwrite (pTrackData, 1, lTrackLen, pFile) < (unsigned long)lTrackLen) {
			free (pTrackData);
			fclose (pFile);
			return 0;
		}
		free (pTrackData);
		pTrackData = NULL;
	}

	/* �g���f�[�^�擪�A�h���X�̏������� */
	fseek (pFile, 512, SEEK_SET);
	if (fwrite (lTempExtendedDataAddress, sizeof (long), 64, pFile) < 64) {
		fclose (pFile);
		return 0;
	}

	/* �g���b�N�擪�A�h���X�̍ď������� */
	fseek (pFile, 768, SEEK_SET);
	if (fwrite (lTempTrackAddress, sizeof (long), usNumTrack, pFile) < usNumTrack) {
		fclose (pFile);
		return 0;
	}

	free (pTrackData);
	pTrackData = NULL;

	fclose (pFile);
	pFile = NULL;

	forEachTrack (pMIDIData, pMIDITrack) {
		pMIDITrack->m_lInputOn = 1;
		pMIDITrack->m_lInputPort = pMIDITrack->m_lOutputPort;
		pMIDITrack->m_lInputChannel = pMIDITrack->m_lOutputChannel;
		pMIDITrack->m_lOutputOn = 1;
	}

	return 1;
}



/* �J���}��؂�̎��̃g�[�N���̐擪�|�C���^���擾���� */
static char* getnexttoken_01 (char* pText) {
	char* p = pText;
	int bInsideDoubleQuat = 0;
	/* ���̃J���}�������ʒu�܂Ń|�C���^��i�߂� */
	/* �_�u���N�H�[�e�[�V������(�������)�̃J���}�͖������� */
	while (1) {
		if (*p == '"') {
			bInsideDoubleQuat = !bInsideDoubleQuat;
		}
		if (*p == ',' && !bInsideDoubleQuat) {
			break;
		}
		else if (*p == '\r' || *p == '\n' || *p == '\0') {
			return p;
		}
		if (0x80 <= *p && *p <= 0x9F) {
			p++;
			p++;
		}
		else {
			p++;
		}
	}
	/* �J���}����Ƀt�H�[�J�X�����킹�� */
	p++;
	/* �J���}����̔��p�X�y�[�X�͓ǂݔ�΂� */
	while (1) {
		if (*p != ' ') {
			break;
		}
		else if (*p == '\r' || *p == '\n' || *p == '\0') {
			return p;
		}
		p++;
	}
	return p;
}

static char* strncpy_ex02 (char* pText1, char* pText2, long lLen1, long lLen2) {
	char* p1 = pText1;
	char* p2 = pText2;
	while (1) {
		/* �P��̃_�u���N�H�[�e�[�V���������� */
		if (p2 - pText2 < lLen2 - 1 && *p2 == '"' && *(p2+1) != '"' ||
			p2 - pText2 == lLen2 - 1 && *p2 == '"') {
			p2++;
		}
		/* 2�A�̃_�u���N�H�[�e�[�V�������P��̃_�u���N�H�[�e�[�V���� */
		if (p1 - pText1 < lLen1 && 
			p2 - pText2 < lLen2 - 1 && *p2 == '"' && *(p2+1) == '"') {
			*p1++ = '"';
			p2++;
			p2++;
		}
		/* 2�o�C�g����(���{��)���P���R�s�[ */
		else if (p1 - pText1 < lLen1 - 1 && 
			p2 - pText2 < lLen2 - 1 && 0x80 <= *p2 && *p2 <= 0x9F) {
			*p1++ = *p2++;
			*p1++ = *p2++;
		}
		/* 1�o�C�g�������P���R�s�[ */
		else if (p1 - pText1 < lLen1 &&	p2 - pText2 < lLen2) {
			*p1++ = *p2++;
		}
		else {
			break;
		}
	}
	return pText1;
}

/* MIDI�f�[�^��MIDICSV�t�@�C��(*.csv)����ǂݍ��� */
/* �V����MIDI�f�[�^�ւ̃|�C���^��Ԃ�(���s��NULL) */
MIDIData* __stdcall MIDIData_LoadFromMIDICSV (const char* pszFileName) {
	FILE* pFile = NULL;
	char szTextLine[2048];
	MIDIData* pMIDIData = NULL;
	MIDITrack* pMIDITrack = NULL;
	long lFormat = 0;
	long lNumTrack = 1;
	long lTimeBase = 1;

	/* �t�@�C�����J�� */
	pFile = fopen (pszFileName, "rt");
	if (pFile == NULL) {
		return NULL;
	}

	/* �t�@�C���I�[�܂�1�s���ǂݍ��� */
	while (!feof (pFile)) {
		char* pTrackIndex = NULL;
		char* pTime = NULL;
		char* pEventKind = NULL;
		long lTrackIndex = 0;
		long lTime = 0;
		long lKind = 0;
		/* 1�s�ǂݎ�� */
		memset (szTextLine, 0, sizeof (szTextLine));
		fgets (szTextLine, sizeof (szTextLine) - 1, pFile);
		/* �R�����g�s�̓ǂݔ�΂� */
		if (szTextLine[0] == '#' || szTextLine[0] == ';') {
			continue;
		}
		/* �g���b�N�ԍ��A�^�C���A�C�x���g�̎�ނ̓ǂݎ�� */
		pTrackIndex = szTextLine;
		pTime = getnexttoken_01 (pTrackIndex);
		pEventKind = getnexttoken_01 (pTime);
		lTrackIndex = atol (szTextLine);
		lTime = atol (pTime);

		/* MIDI�f�[�^�w�b�_���s */
		if (lTrackIndex == 0 && lTime == 0 && strnicmp (pEventKind, "Header", 6) == 0) {
			long lTimeMode = MIDIDATA_TPQNBASE;
			long lTimeResolution = 120;
			char* pFormat = getnexttoken_01 (pEventKind);
			char* pNumTrack = getnexttoken_01 (pFormat);
			char* pTimeBase = getnexttoken_01 (pNumTrack);
			lFormat = atol (pFormat);
			lNumTrack = atol (pNumTrack);
			lTimeBase = atol (pTimeBase);
			if (lFormat < 0 || lFormat > 2) {
				return NULL;
			}
			if (lNumTrack < 1 || lNumTrack > 65536) {
				return NULL;
			}
			if (0 < lTimeBase && lTimeBase <= 960) {
				lTimeMode = MIDIDATA_TPQNBASE;
				lTimeResolution = lTimeBase;
			}
			else if (lTimeBase & 0x8000) {
				switch (256 - ((lTimeBase & 0xFF00) >> 8)) {
				case 24:
					lTimeMode = MIDIDATA_SMPTE24BASE;
					lTimeResolution = lTimeBase & 0xFF;
					break;
				case 25:
					lTimeMode = MIDIDATA_SMPTE25BASE;
					lTimeResolution = lTimeBase & 0xFF;
					break;
				case 29:
					lTimeMode = MIDIDATA_SMPTE29BASE;
					lTimeResolution = lTimeBase & 0xFF;
					break;
				case 30:
					lTimeMode = MIDIDATA_SMPTE30BASE;
					lTimeResolution = lTimeBase & 0xFF;
					break;
				}
			}
			else {
				break;
			}
			/* ��d��MIDI�f�[�^����낤�Ƃ����ꍇ�A�����ɓǂݍ��ݒ��f */
			if (pMIDIData != NULL) {
				break;
			}
			/* MIDI�f�[�^�̍쐬(SMPTE�x�[�X�̓T�|�[�g���Ȃ�) */
			pMIDIData = MIDIData_Create (lFormat, lNumTrack, lTimeMode, lTimeResolution);
			if (pMIDIData == NULL) {
				return NULL;
			}
		}

		/* �X�^�[�g�I�u�g���b�N */
		if (lTime == 0 && strnicmp (pEventKind, "Start_track", 11) == 0) {
			/* ��d��MIDI�f�[�^����낤�Ƃ����ꍇ�A�����ɓǂݍ��ݒ��f */
			if (pMIDITrack != NULL) {
				break;
			}
			/* ���̃Z�N�V������MIDI�g���b�N�̎擾 */
			if (pMIDIData && 1 <= lTrackIndex && lTrackIndex <= lNumTrack) {
				pMIDITrack = MIDIData_GetTrack (pMIDIData, lTrackIndex - 1);
			}
		}

		/* �G���h�I�u�g���b�N */
		if (strnicmp (pEventKind, "End_track", 9) == 0) {
			if (pMIDIData && pMIDITrack) {
				MIDITrack_InsertEndofTrack (pMIDITrack, lTime);
			}
			pMIDITrack = NULL;
		}

		/* �V�[�P���X�ԍ� */
		if (strnicmp (pEventKind, "Sequence_number", 15) == 0) {
			if (pMIDIData && pMIDITrack) {
				char* pNumber = getnexttoken_01 (pEventKind);
				long lNumber = CLIP (0, atol (pNumber), 65535);
				MIDITrack_InsertSequenceNumber (pMIDITrack, lTime, lNumber);
			}
		}

		/* �e�L�X�g */
		if (strnicmp (pEventKind, "Text_t", 6) == 0) {
			if (pMIDIData && pMIDITrack) {
				char* pText = getnexttoken_01 (pEventKind);
				char szText[1024];
				memset (szText, 0, sizeof (szText));
				strncpy_ex02 (szText, pText, sizeof (szText) - 1, strlen (pText));
				MIDITrack_InsertTextEvent (pMIDITrack, lTime, szText);
			}
		}
		
		/* ���쌠 */
		if (strnicmp (pEventKind, "Copyright_t", 11) == 0) {
			if (pMIDIData && pMIDITrack) {
				char* pText = getnexttoken_01 (pEventKind);
				char szText[1024];
				memset (szText, 0, sizeof (szText));
				strncpy_ex02 (szText, pText, sizeof (szText) - 1, strlen (pText));
				MIDITrack_InsertCopyrightNotice (pMIDITrack, lTime, szText);
			}
		}

		/* �g���b�N���^�V�[�P���X�� */
		if (strnicmp (pEventKind, "Title_t", 7) == 0) {
			if (pMIDIData && pMIDITrack) {
				char* pText = getnexttoken_01 (pEventKind);
				char szText[1024];
				memset (szText, 0, sizeof (szText));
				strncpy_ex02 (szText, pText, sizeof (szText) - 1, strlen (pText));
				MIDITrack_InsertTrackName (pMIDITrack, lTime, szText);
			}
		}

		/* �C���X�g�D�������g�� */
		if (strnicmp (pEventKind, "Instrument_name_t", 17) == 0) {
			if (pMIDIData && pMIDITrack) {
				char* pText = getnexttoken_01 (pEventKind);
				char szText[1024];
				memset (szText, 0, sizeof (szText));
				strncpy_ex02 (szText, pText, sizeof (szText) - 1, strlen (pText));
				MIDITrack_InsertInstrumentName (pMIDITrack, lTime, szText);
			}
		}

		/* �̎� */
		if (strnicmp (pEventKind, "Lyric_t", 7) == 0) {
			if (pMIDIData && pMIDITrack) {
				char* pText = getnexttoken_01 (pEventKind);
				char szText[1024];
				memset (szText, 0, sizeof (szText));
				strncpy_ex02 (szText, pText, sizeof (szText) - 1, strlen (pText));
				MIDITrack_InsertLyric (pMIDITrack, lTime, szText);
			}
		}

		/* �}�[�J�[ */
		if (strnicmp (pEventKind, "Marker_t", 8) == 0) {
			if (pMIDIData && pMIDITrack) {
				char* pText = getnexttoken_01 (pEventKind);
				char szText[1024];
				memset (szText, 0, sizeof (szText));
				strncpy_ex02 (szText, pText, sizeof (szText) - 1, strlen (pText));
				MIDITrack_InsertMarker (pMIDITrack, lTime, szText);
			}
		}

		/* �L���[�|�C���g */
		if (strnicmp (pEventKind, "Cue_point_t", 11) == 0) {
			if (pMIDIData && pMIDITrack) {
				char* pText = getnexttoken_01 (pEventKind);
				char szText[1024];
				memset (szText, 0, sizeof (szText));
				strncpy_ex02 (szText, pText, sizeof (szText) - 1, strlen (pText));
				MIDITrack_InsertCuePoint (pMIDITrack, lTime, szText);
			}
		}

		/* �v���O������(�����Ȃ�) */

		/* �f�o�C�X��(�����Ȃ�) */

		/* �|�[�g�v���t�B�b�N�X */
		if (strnicmp (pEventKind, "MIDI_port", 9) == 0) {
			if (pMIDIData && pMIDITrack) {
				char* pNumber = getnexttoken_01 (pEventKind);
				long lNumber = CLIP (0, atol (pNumber), 255);
				MIDITrack_InsertPortPrefix (pMIDITrack, lTime, lNumber);
			}
		}

		/* �`�����l���v���t�B�b�N�X */
		if (strnicmp (pEventKind, "Channel_prefix", 14) == 0) {
			if (pMIDIData && pMIDITrack) {
				char* pNumber = getnexttoken_01 (pEventKind);
				long lNumber = CLIP (0, atol (pNumber), 15);
				MIDITrack_InsertChannelPrefix (pMIDITrack, lTime, lNumber);
			}
		}


		/* �e���| */
		if (strnicmp (pEventKind, "Tempo", 5) == 0) {
			if (pMIDIData && pMIDITrack) {
				char* pTempo = getnexttoken_01 (pEventKind);
				long lTempo = CLIP (1, atol (pTempo), 60000000);
				MIDITrack_InsertTempo (pMIDITrack, lTime, lTempo);
			}
		}

		/* SMPTE�I�t�Z�b�g */
		if (strnicmp (pEventKind, "SMPTE_offset", 12) == 0) {
			if (pMIDIData && pMIDITrack) {
				char* pHour = getnexttoken_01 (pEventKind);
				char* pMinute = getnexttoken_01 (pHour);
				char* pSecond = getnexttoken_01 (pMinute);
				char* pFrame = getnexttoken_01 (pSecond);
				char* pSubFrame = getnexttoken_01 (pFrame);
				long lRate = CLIP (0, (atol (pHour) & 0x60) >> 5, 3);
				long lHour = CLIP (0, atol (pHour) & 0x1F, 23);
				long lMinute = CLIP (0, atol (pMinute), 59);
				long lSecond = CLIP (0, atol (pSecond), 59);
				long lFrame = CLIP (0, atol (pFrame), 29);
				long lSubFrame = CLIP (0, atol (pSubFrame), 99);
				MIDITrack_InsertSMPTEOffset (pMIDITrack, lTime, lRate, lHour, lMinute, lSecond, lFrame, lSubFrame);
			}
		}

		/* ���q�L�� */
		if (strnicmp (pEventKind, "Time_signature", 14) == 0) {
			if (pMIDIData && pMIDITrack) {
				char* pN = getnexttoken_01 (pEventKind);
				char* pD = getnexttoken_01 (pN);
				char* pC = getnexttoken_01 (pD);
				char* pB = getnexttoken_01 (pC);
				long lN = CLIP (1, atol (pN), 127);
				long lD = CLIP (1, atol (pD), 8);
				long lC = CLIP (1, atol (pC), 127); /* ���24������ */
				long lB = CLIP (1, atol (pB), 127); /* ���8������ */
				MIDITrack_InsertTimeSignature (pMIDITrack, lTime, lN, lD, lC, lB);
			}
		}

		/* �����L�� */
		if (strnicmp (pEventKind, "Key_signature", 13) == 0) {
			if (pMIDIData && pMIDITrack) {
				char* pTone = getnexttoken_01 (pEventKind);
				char* pMinor = getnexttoken_01 (pTone);
				long lTone = CLIP (-7, atol (pTone), 7);
				long lMinor = strnicmp (pMinor, "\"minor\"", 7) == 0 ? 1 : 0;
				MIDITrack_InsertKeySignature (pMIDITrack, lTime, lTone, lMinor);
			}
		}

		/* �V�[�P���T�[�Ǝ��̃C�x���g */
		if (strnicmp (pEventKind, "Sequencer_specific", 18) == 0) {
			if (pMIDIData && pMIDITrack) {
				long k = 0;
				unsigned char ucData[1024];
				char* pLen = getnexttoken_01 (pEventKind);
				char* pData = getnexttoken_01 (pLen);
				long lLen = CLIP (0, atol (pLen), sizeof (ucData));
				memset (ucData, 0, sizeof (ucData));
				while (*pData != '\r' && *pData != '\n' && *pData != '\0' && k < lLen) { 
					ucData[k++] = (unsigned char)atoi (pData);
					pData = getnexttoken_01 (pData);
				}
				MIDITrack_InsertSequencerSpecific (pMIDITrack, lTime, ucData, lLen);
			}
		}

		/* ����`�̃��^�C�x���g */
		if (strnicmp (pEventKind, "Unknown_meta_event", 18) == 0) {
			if (pMIDIData && pMIDITrack) {
				long k = 0;
				unsigned char ucData[1024];
				char* pKind = getnexttoken_01 (pEventKind);
				char* pLen = getnexttoken_01 (pKind);
				char* pData = getnexttoken_01 (pLen);
				long lKind = atol (pKind);
				if (0x00 <= lKind && lKind <= 0x7F) {
					MIDIEvent* pMIDIEvent = NULL;
					long lLen = CLIP (0, atol (pLen), sizeof (ucData));
					memset (ucData, 0, sizeof (ucData));
					while (*pData != '\r' && *pData != '\n' && *pData != '\0' && k < lLen) { 
						ucData[k++] = (unsigned char)atoi (pData);
						pData = getnexttoken_01 (pData);
					}
					pMIDIEvent = MIDIEvent_Create (lTime, lKind, ucData, lLen);
					if (pMIDIEvent) {
						MIDITrack_InsertEvent (pMIDITrack, pMIDIEvent);
					}
				}
			}
		}

		/* �m�[�g�I�t */
		if (strnicmp (pEventKind, "Note_off_c", 10) == 0) {
			if (pMIDIData && pMIDITrack) {
				char* pChannel = getnexttoken_01 (pEventKind);
				char* pKey = getnexttoken_01 (pChannel);
				char* pVelocity = getnexttoken_01 (pKey);
				long lChannel = CLIP (0, atol (pChannel), 15);
				long lKey = CLIP (0, atol (pKey), 127);
				long lVelocity = CLIP (0, atol (pVelocity), 127);
				MIDITrack_InsertNoteOff (pMIDITrack, lTime, lChannel, lKey, lVelocity);
			}
		}

		/* �m�[�g�I�� */
		if (strnicmp (pEventKind, "Note_on_c", 9) == 0) {
			if (pMIDIData && pMIDITrack) {
				char* pChannel = getnexttoken_01 (pEventKind);
				char* pKey = getnexttoken_01 (pChannel);
				char* pVelocity = getnexttoken_01 (pKey);
				long lChannel = CLIP (0, atol (pChannel), 15);
				long lKey = CLIP (0, atol (pKey), 127);
				long lVelocity = CLIP (0, atol (pVelocity), 127);
				MIDITrack_InsertNoteOn (pMIDITrack, lTime, lChannel, lKey, lVelocity);
			}
		}

		/* �L�[�A�t�^�[�^�b�` */
		if (strnicmp (pEventKind, "Poly_aftertouch_c", 17) == 0) {
			if (pMIDIData && pMIDITrack) {
				char* pChannel = getnexttoken_01 (pEventKind);
				char* pKey = getnexttoken_01 (pChannel);
				char* pValue = getnexttoken_01 (pKey);
				long lChannel = CLIP (0, atol (pChannel), 15);
				long lKey = CLIP (0, atol (pKey), 127);
				long lValue = CLIP (0, atol (pValue), 127);
				MIDITrack_InsertKeyAftertouch (pMIDITrack, lTime, lChannel, lKey, lValue);
			}
		}

		/* �R���g���[���`�F���W */
		if (strnicmp (pEventKind, "Control_c", 9) == 0) {
			if (pMIDIData && pMIDITrack) {
				char* pChannel = getnexttoken_01 (pEventKind);
				char* pNumber = getnexttoken_01 (pChannel);
				char* pValue = getnexttoken_01 (pNumber);
				long lChannel = CLIP (0, atol (pChannel), 15);
				long lNumber = CLIP (0, atol (pNumber), 127);
				long lValue = CLIP (0, atol (pValue), 127);
				MIDITrack_InsertControlChange (pMIDITrack, lTime, lChannel, lNumber, lValue);
			}
		}

		/* �v���O�����`�F���W */
		if (strnicmp (pEventKind, "Program_c", 9) == 0) {
			if (pMIDIData && pMIDITrack) {
				char* pChannel = getnexttoken_01 (pEventKind);
				char* pNumber = getnexttoken_01 (pChannel);
				long lChannel = CLIP (0, atol (pChannel), 15);
				long lNumber = CLIP (0, atol (pNumber), 127);
				MIDITrack_InsertProgramChange (pMIDITrack, lTime, lChannel, lNumber);
			}
		}

		/* �`�����l���A�t�^�[�^�b�` */
		if (strnicmp (pEventKind, "Channel_aftertouch_c", 20) == 0) {
			if (pMIDIData && pMIDITrack) {
				char* pChannel = getnexttoken_01 (pEventKind);
				char* pValue = getnexttoken_01 (pChannel);
				long lChannel = CLIP (0, atol (pChannel), 15);
				long lValue = CLIP (0, atol (pValue), 127);
				MIDITrack_InsertChannelAftertouch (pMIDITrack, lTime, lChannel, lValue);
			}
		}

		/* �s�b�`�x���h */
		if (strnicmp (pEventKind, "Pitch_bend_c", 12) == 0) {
			if (pMIDIData && pMIDITrack) {
				char* pChannel = getnexttoken_01 (pEventKind);
				char* pValue = getnexttoken_01 (pChannel);
				long lChannel = CLIP (0, atol (pChannel), 15);
				long lValue = CLIP (0, atol (pValue), 16363);
				MIDITrack_InsertPitchBend (pMIDITrack, lTime, lChannel, lValue);
			}
		}

		/* �V�X�e���G�N�X�N���[�V�� */
		if (strnicmp (pEventKind, "System_exclusive", 16) == 0 &&
			strnicmp (pEventKind, "System_exclusive_packet", 23) != 0) {
			if (pMIDIData && pMIDITrack) {
				long k = 1;
				unsigned char ucData[1024];
				char* pLen = getnexttoken_01 (pEventKind);
				char* pData = getnexttoken_01 (pLen);
				long lLen = CLIP (0, atol (pLen), sizeof (ucData));
				memset (ucData, 0, sizeof (ucData));
				ucData[0] = 0xF0;
				while (*pData != '\r' && *pData != '\n' && *pData != '\0' && k < lLen + 1) { 
					ucData[k++] = (unsigned char)atoi (pData);
					pData = getnexttoken_01 (pData);
				}
				MIDITrack_InsertSysExEvent (pMIDITrack, lTime, ucData, lLen + 1);
			}
		}

		/* �V�X�e���G�N�X�N���[�V��(����) */
		if (strnicmp (pEventKind, "System_exclusive_packet", 23) == 0) {
			if (pMIDIData && pMIDITrack) {
				long k = 0;
				unsigned char ucData[1024];
				char* pLen = getnexttoken_01 (pEventKind);
				char* pData = getnexttoken_01 (pLen);
				long lLen = CLIP (0, atol (pLen), sizeof (ucData));
				memset (ucData, 0, sizeof (ucData));
				while (*pData != '\r' && *pData != '\n' && *pData != '\0' && k < lLen) { 
					ucData[k++] = (unsigned char)atoi (pData);
					pData = getnexttoken_01 (pData);
				}
				MIDITrack_InsertSysExEvent (pMIDITrack, lTime, ucData, lLen);
			}
		}
	}

	/* �t�@�C������� */
	fclose (pFile);

	/* �G���h�I�u�g���b�N��t���Y��Ă���g���b�N�͎����I�ɕt������ */
	if (pMIDIData) {
		forEachTrack (pMIDIData, pMIDITrack) {
			MIDIEvent* pLastEvent = MIDITrack_GetLastEvent (pMIDITrack);
			if (!MIDIEvent_IsEndofTrack (pLastEvent)) {
				MIDITrack_InsertEndofTrack (pMIDITrack, pLastEvent->m_lTime);
			}
		}
	}

	/* �e�g���b�N�̏o�̓|�[�g�ԍ��E�o�̓`�����l���E�\�����[�h�̎����ݒ� */
	MIDIData_UpdateOutputPort (pMIDIData);
	MIDIData_UpdateOutputChannel (pMIDIData);
	MIDIData_UpdateViewMode (pMIDIData);

	/* �e�g���b�N�̓��̓|�[�g�ԍ��E���̓`�����l���̐ݒ� */
	forEachTrack (pMIDIData, pMIDITrack) {
		pMIDITrack->m_lInputOn = 1;
		pMIDITrack->m_lInputPort = pMIDITrack->m_lOutputPort;
		pMIDITrack->m_lInputChannel = pMIDITrack->m_lOutputChannel;
		pMIDITrack->m_lOutputOn = 1;
	}

	return pMIDIData;
}


/* strncpy�̓����01("��"",���s13��\n�A���s10��\r�ɕϊ�)(�B��) */
static char* strncpy_ex01 (char* pText1, char* pText2, long lLen1, long lLen2) {
	char* p1 = pText1;
	char* p2 = pText2;
	while (1) {
		if (p1 - pText1 < lLen1 - 1 && 
			p2 - pText2 < lLen2 && *p2 == '"') {
			*p1++ = '\"';
			*p1++ = '\"';
			p2++;
		}
		else if (p1 - pText1 < lLen1 - 1 && 
			p2 - pText2 < lLen2 && *p2 == '\r') {
			*p1++ = '\\';
			*p1++ = 'r';
			p2++;
		}
		else if (p1 - pText1 < lLen1 - 1 && 
			p2 - pText2 < lLen2 && *p2 == '\n') {
			*p1++ = '\\';
			*p1++ = 'n';
			p2++;
		}
		else if (p1 - pText1 < lLen1 - 1 &&
			p2 - pText2 < lLen2 - 1 && 0x80 <= *p2 && *p2 <= 0x9F) {
			*p1++ = *p2++;
			*p1++ = *p2++;
		}
		else if (p1 - pText1 < lLen1 && p2 - pText2 < lLen2) {
			*p1++ = *p2++;
		}
		else {
			break;
		}
	}
	return pText1;
}

/* MIDICSV�`���ŃC�x���g��1�t�@�C���ɕۑ�����(�B��) */
static long MIDIEvent_SaveAsMIDICSV (MIDIEvent* pMIDIEvent, long lTrackIndex, FILE* pFile) {
	long lWriteLen = 0;
	long lTime = pMIDIEvent->m_lTime;
	long k = 0;
	if (MIDIEvent_IsMetaEvent (pMIDIEvent)) {
		char szText[1024];
		memset (szText, 0, sizeof (szText));
		switch (pMIDIEvent->m_lKind) {
		case MIDIEVENT_SEQUENCENUMBER:
			lWriteLen = fprintf (pFile, "%ld, %ld, Sequence_number, %ld\n", 
				lTrackIndex + 1, lTime,	MIDIEvent_GetNumber (pMIDIEvent));
			break;
		case MIDIEVENT_TEXTEVENT:
			strncpy_ex01 (szText, pMIDIEvent->m_pData, sizeof (szText) - 1, pMIDIEvent->m_lLen);
			lWriteLen = fprintf (pFile, "%ld, %ld, Text_t, \"%s\"\n", 
				lTrackIndex + 1, lTime, szText);
			break;
		case MIDIEVENT_COPYRIGHTNOTICE:
			strncpy_ex01 (szText, pMIDIEvent->m_pData, sizeof (szText) - 1, pMIDIEvent->m_lLen);
			lWriteLen = fprintf (pFile, "%ld, %ld, Copyright_t, \"%s\"\n", 
				lTrackIndex + 1, lTime, szText);
			break;
		case MIDIEVENT_TRACKNAME:
			strncpy_ex01 (szText, pMIDIEvent->m_pData, sizeof (szText) - 1, pMIDIEvent->m_lLen);
			lWriteLen = fprintf (pFile, "%ld, %ld, Title_t, \"%s\"\n", 
				lTrackIndex + 1, lTime, szText);
			break;
		case MIDIEVENT_INSTRUMENTNAME:
			strncpy_ex01 (szText, pMIDIEvent->m_pData, sizeof (szText) - 1, pMIDIEvent->m_lLen);
			lWriteLen = fprintf (pFile, "%ld, %ld, Instrument_name_t, \"%s\"\n", 
				lTrackIndex + 1, lTime, szText);
			break;
		case MIDIEVENT_LYRIC:
			strncpy_ex01 (szText, pMIDIEvent->m_pData, sizeof (szText) - 1, pMIDIEvent->m_lLen);
			lWriteLen = fprintf (pFile, "%ld, %ld, Lyric_t, \"%s\"\n", 
				lTrackIndex + 1, lTime, szText);
			break;
		case MIDIEVENT_MARKER:
			strncpy_ex01 (szText, pMIDIEvent->m_pData, sizeof (szText) - 1, pMIDIEvent->m_lLen);
			lWriteLen = fprintf (pFile, "%ld, %ld, Marker_t, \"%s\"\n", 
				lTrackIndex + 1, lTime, szText);
			break;
		case MIDIEVENT_CUEPOINT:
			strncpy_ex01 (szText, pMIDIEvent->m_pData, sizeof (szText) - 1, pMIDIEvent->m_lLen);
			lWriteLen = fprintf (pFile, "%ld, %ld, Cue_point_t, \"%s\"\n", 
				lTrackIndex + 1, lTime, szText);
			break;
		/*case MIDIEVENT_PROGRAMNAME: */
		/*	break; */
		/*case MIDIEVENT_DEVICENAME: */
		/*	break; */
		case MIDIEVENT_CHANNELPREFIX:
			lWriteLen = fprintf (pFile, "%ld, %ld, Channel_prefix, %ld\n", 
				lTrackIndex + 1, lTime,	MIDIEvent_GetNumber (pMIDIEvent));
			break;
		case MIDIEVENT_PORTPREFIX:
			lWriteLen = fprintf (pFile, "%ld, %ld, MIDI_port, %ld\n", 
				lTrackIndex + 1, lTime, MIDIEvent_GetNumber (pMIDIEvent));
			break;
		case MIDIEVENT_ENDOFTRACK:
			break;
		case MIDIEVENT_TEMPO:
			lWriteLen = fprintf (pFile, "%ld, %ld, Tempo, %ld\n", 
				lTrackIndex + 1, lTime,	MIDIEvent_GetTempo (pMIDIEvent));
			break;
		case MIDIEVENT_SMPTEOFFSET:
			lWriteLen = fprintf (pFile, "%ld, %ld, SMPTE_offset, %ld, %ld, %ld, %ld, %ld\n", 
				lTrackIndex + 1, lTime,
				pMIDIEvent->m_pData[0], pMIDIEvent->m_pData[1], 
				pMIDIEvent->m_pData[2], pMIDIEvent->m_pData[3],
				pMIDIEvent->m_pData[4]);
			break;
		case MIDIEVENT_TIMESIGNATURE:
			lWriteLen = fprintf (pFile, "%ld, %ld, Time_signature, %ld, %ld, %ld, %ld\n", 
				lTrackIndex + 1, lTime,
				pMIDIEvent->m_pData[0], pMIDIEvent->m_pData[1], 
				pMIDIEvent->m_pData[2], pMIDIEvent->m_pData[3]);
			break;
		case MIDIEVENT_KEYSIGNATURE:
			lWriteLen = fprintf (pFile, "%ld, %ld, Key_signature, %ld, \"%s\"\n", 
				lTrackIndex + 1, lTime,
				CLIP (-7, (char)pMIDIEvent->m_pData[0], 7),
				pMIDIEvent->m_pData[1] ? "minor" : "major");
			break;
		case MIDIEVENT_SEQUENCERSPECIFIC:
			lWriteLen = fprintf (pFile, "%ld, %ld, Sequencer_specific, %ld",
				lTrackIndex + 1, lTime, pMIDIEvent->m_lLen);
			for (k = 0; k < pMIDIEvent->m_lLen; k++) {
				lWriteLen = fprintf (pFile, ", %ld", pMIDIEvent->m_pData[k]);
			}
			lWriteLen = fprintf (pFile, "\n");
			break;
		default:
			lWriteLen = fprintf (pFile, "%ld, %ld, Unknown_meta_event, %ld, %ld",
				lTrackIndex + 1, lTime, pMIDIEvent->m_lKind, pMIDIEvent->m_lLen);
			for (k = 0; k < pMIDIEvent->m_lLen; k++) {
				lWriteLen = fprintf (pFile, ", %ld", pMIDIEvent->m_pData[k]);
			}
			lWriteLen = fprintf (pFile, "\n");
			break;
		}
	}
	else if (MIDIEvent_IsMIDIEvent (pMIDIEvent)) {
		switch (pMIDIEvent->m_lKind & 0xF0) {
		case MIDIEVENT_NOTEOFF:
			lWriteLen = fprintf (pFile, "%ld, %ld, Note_off_c, %ld, %ld, %ld\n", 
				lTrackIndex + 1, lTime, pMIDIEvent->m_pData[0] & 0x0F,
				pMIDIEvent->m_pData[1], pMIDIEvent->m_pData[2]);
			break;
		case MIDIEVENT_NOTEON:
			lWriteLen = fprintf (pFile, "%ld, %ld, Note_on_c, %ld, %ld, %ld\n", 
				lTrackIndex + 1, lTime, pMIDIEvent->m_pData[0] & 0x0F,
				pMIDIEvent->m_pData[1], pMIDIEvent->m_pData[2]);
			break;
		case MIDIEVENT_KEYAFTERTOUCH:
			lWriteLen = fprintf (pFile, "%ld, %ld, Poly_aftertouch_c, %ld, %ld, %ld\n", 
				lTrackIndex + 1, lTime, pMIDIEvent->m_pData[0] & 0x0F,
				pMIDIEvent->m_pData[1], pMIDIEvent->m_pData[2]);
			break;
		case MIDIEVENT_CONTROLCHANGE:
			lWriteLen = fprintf (pFile, "%ld, %ld, Control_c, %ld, %ld, %ld\n", 
				lTrackIndex + 1, lTime, pMIDIEvent->m_pData[0] & 0x0F,
				pMIDIEvent->m_pData[1], pMIDIEvent->m_pData[2]);
			break;
		case MIDIEVENT_PROGRAMCHANGE:
			lWriteLen = fprintf (pFile, "%ld, %ld, Program_c, %ld, %ld\n", 
				lTrackIndex + 1, lTime, pMIDIEvent->m_pData[0] & 0x0F,
				pMIDIEvent->m_pData[1], pMIDIEvent->m_pData[2]);
			break;
		case MIDIEVENT_CHANNELAFTERTOUCH:
			lWriteLen = fprintf (pFile, "%ld, %ld, Channel_aftertouch_c, %ld, %ld\n", 
				lTrackIndex + 1, lTime, pMIDIEvent->m_pData[0] & 0x0F,
				pMIDIEvent->m_pData[1], pMIDIEvent->m_pData[2]);
			break;
		case MIDIEVENT_PITCHBEND:
			lWriteLen = fprintf (pFile, "%ld, %ld, Pitch_bend_c, %ld, %ld\n", 
				lTrackIndex + 1, lTime, pMIDIEvent->m_pData[0] & 0x0F,
				pMIDIEvent->m_pData[2] * 128 + pMIDIEvent->m_pData[1]);
			break;
		}
	}
	else if (MIDIEvent_IsSysExEvent (pMIDIEvent)) {
		switch (pMIDIEvent->m_lKind) {
		case MIDIEVENT_SYSEXSTART:
			lWriteLen = fprintf (pFile, "%ld, %ld, System_exclusive, %ld",
				lTrackIndex + 1, lTime, pMIDIEvent->m_lLen - 1);
			for (k = 1; k < pMIDIEvent->m_lLen; k++) {
				lWriteLen = fprintf (pFile, ", %ld", pMIDIEvent->m_pData[k]);
			}
			lWriteLen = fprintf (pFile, "\n");
			break;
		case MIDIEVENT_SYSEXCONTINUE:
			lWriteLen = fprintf (pFile, "%ld, %ld, System_exclusive_packet, %ld",
				lTrackIndex + 1, lTime, pMIDIEvent->m_lLen);
			for (k = 0; k < pMIDIEvent->m_lLen; k++) {
				lWriteLen = fprintf (pFile, ", %ld", pMIDIEvent->m_pData[k]);
			}
			lWriteLen = fprintf (pFile, "\n");
			break;
		}
	}
	return lWriteLen;
}

/* MIDI�f�[�^��MIDICSV�t�@�C��(*.csv)�Ƃ��ĕۑ� */
long __stdcall MIDIData_SaveAsMIDICSV (MIDIData* pMIDIData, const char* pszFileName) {
	long lWriteLen = 0;
	MIDITrack* pMIDITrack = NULL;
	MIDIEvent* pMIDIEvent = NULL;
	FILE* pFile = NULL;
	long i = 0;
	long lXFVersion = MIDIData_GetXFVersion (pMIDIData);

	/* �t�@�C�����J�� */
	pFile = fopen (pszFileName, "wt");
	if (pFile == NULL) {
		return 0;
	}

	/* �w�b�_�[�̕ۑ� */
	lWriteLen = fprintf (pFile, "0, 0, Header, %d, %d, %d\n",
		(unsigned short)(pMIDIData->m_lFormat),
		(unsigned short)(pMIDIData->m_lNumTrack),
		(short)(pMIDIData->m_lTimeBase));
	if (lWriteLen == 0) {
		fclose (pFile);
		return 0;
	}

	/* �e�g���b�N�̕ۑ� */
	i = 0;
	forEachTrack (pMIDIData, pMIDITrack) {
		long lLastTime = 0;
		if (pMIDITrack->m_pLastEvent) {
			lLastTime = pMIDITrack->m_pLastEvent->m_lTime;
		}
		/* �g���b�N�w�b�_�[�̕ۑ� */
		lWriteLen = fprintf (pFile, "%ld, 0, Start_track\n", i + 1);
		if (lWriteLen == 0) {
			fclose (pFile);
			return 0;
		}
		/* �e�C�x���g�̕ۑ�(EndofTrack������) */
		forEachEvent (pMIDITrack, pMIDIEvent) {
			lWriteLen = MIDIEvent_SaveAsMIDICSV (pMIDIEvent, i, pFile);
		}
		/* �g���b�N�t�b�^�[�̕ۑ� */
		lWriteLen = fprintf (pFile, "%ld, %ld, End_track\n", i + 1, lLastTime);
		if (lWriteLen == 0) {
			fclose (pFile);
			return 0;
		}
		i++;
	}


	/* �t�b�^�[�̕ۑ� */
	lWriteLen = fprintf (pFile, "0, 0, End_of_file\n");
	if (lWriteLen == 0) {
		fclose (pFile);
		return 0;
	}

	fclose (pFile);
	return 1;
}

