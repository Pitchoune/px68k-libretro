// ---------------------------------------------------------------------------
//	OPN/A/B interface with ADPCM support
//	Copyright (C) cisc 1998, 2003.
// ---------------------------------------------------------------------------
//	$fmgen-Id: opna.h,v 1.33 2003/06/12 13:14:37 cisc Exp $

#ifndef FM_OPNA_H
#define FM_OPNA_H

#include <stdint.h>
#include "fmgen.h"
#include "fmtimer.h"
#include "psg.h"

namespace FM
{
	//	OPN Base -------------------------------------------------------
	class OPNBase : public Timer
	{
	public:
		OPNBase();
		virtual ~OPNBase() {}
		
		bool	Init(uint32_t c, uint32_t r);
		virtual void Reset();
		
		void	SetVolumeFM(int db);
		void	SetVolumePSG(int db);

	protected:
		void	SetParameter(Channel4* ch, uint32_t addr, uint32_t data);
		void	SetPrescaler(uint32_t p);
		void	RebuildTimeTable();
		
		int		fmvolume;
		
		uint32_t	clock;				// OPN クロック
		uint32_t	rate;				// FM 音源合成レート
		uint32_t	psgrate;			// FMGen  出力レート
		uint32_t	status;
		Channel4* csmch;
		

		static  uint32_t lfotable[8];
	
	private:
		void	TimerA();
		uint8_t	prescale;
		
	protected:
		Chip	chip;
		PSG		psg;
	};

	//	OPN2 Base ------------------------------------------------------
	class OPNABase : public OPNBase
	{
	public:
		OPNABase();
		virtual ~OPNABase();
		
		uint32_t	ReadStatus() { return status & 0x03; }
		uint32_t	ReadStatusEx();
		void	SetChannelMask(uint32_t mask);
	
	private:
		virtual void Intr(bool) {}

		void	MakeTable2();
	
	protected:
		bool	Init(uint32_t c, uint32_t r);
		bool	SetRate(uint32_t c, uint32_t r);

		void	Reset();
		void 	SetReg(uint32_t addr, uint32_t data);
		void	SetADPCMBReg(uint32_t reg, uint32_t data);
		uint32_t	GetReg(uint32_t addr);	
	
	protected:
		void	FMMix(int16_t* buffer, size_t nsamples);
		void 	Mix6(int16_t* buffer, size_t nsamples, int activech);
		
		void	MixSubS(int activech, ISample**);
		void	MixSubSL(int activech, ISample**);

		void	SetStatus(uint32_t bit);
		void	ResetStatus(uint32_t bit);
		void	UpdateStatus();
		void	LFO();

		void	DecodeADPCMB();
		void	ADPCMBMix(int16_t* dest, size_t count);

		void	WriteRAM(uint32_t data);
		uint32_t	ReadRAM();
		int		ReadRAMN();
		int		DecodeADPCMBSample(uint32_t);
		
	// FM 音源関係
		uint8_t	pan[6];
		uint8_t	fnum2[9];
		
		uint8_t	reg22;
		uint32_t	reg29;		// OPNA only?
		
		uint32_t	stmask;
		uint32_t	statusnext;

		uint32_t	lfocount;
		uint32_t	lfodcount;
		
		uint32_t	fnum[6];
		uint32_t	fnum3[3];
		
	// ADPCM 関係
		uint8_t*	adpcmbuf;		// ADPCM RAM
		uint32_t	adpcmmask;		// メモリアドレスに対するビットマスク
		uint32_t	adpcmnotice;	// ADPCM 再生終了時にたつビット
		uint32_t	startaddr;		// Start address
		uint32_t	stopaddr;		// Stop address
		uint32_t	memaddr;		// 再生中アドレス
		uint32_t	limitaddr;		// Limit address/mask
		int		adpcmlevel;		// ADPCM 音量
		int		adpcmvolume;
		int		adpcmvol;
		uint32_t	deltan;			// ��N
		int		adplc;			// 周波数変換用変数
		int		adpld;			// 周波数変換用変数差分値
		uint32_t	adplbase;		// adpld の元
		int		adpcmx;			// ADPCM 合成用 x
		int		adpcmd;			// ADPCM 合成用 ��
		int		adpcmout;		// ADPCM 合成後の出力
		int		apout0;			// out(t-2)+out(t-1)
		int		apout1;			// out(t-1)+out(t)

		uint32_t	adpcmreadbuf;	// ADPCM リード用バッファ
		bool	adpcmplay;		// ADPCM 再生中
		int8_t	granuality;		
		bool	adpcmmask_;

		uint8_t	control1;		// ADPCM コントロールレジスタ１
		uint8_t	control2;		// ADPCM コントロールレジスタ２
		uint8_t	adpcmreg[8];	// ADPCM レジスタの一部分

		int		rhythmmask_;

		Channel4 ch[6];

		static void	BuildLFOTable();
		static int amtable[FM_LFOENTS];
		static int pmtable[FM_LFOENTS];
		static int32_t tltable[FM_TLENTS+FM_TLPOS];
		static bool	tablehasmade;
	};

	//	YM2203(OPN) ----------------------------------------------------
	class OPN : public OPNBase
	{
	public:
		OPN();
		virtual ~OPN() {}
		
		bool	Init(uint32_t c, uint32_t r, const char*);
		bool	SetRate(uint32_t c, uint32_t r);
		
		void	Reset();
		void 	Mix(int16_t* buffer, size_t nsamples);
		void 	SetReg(uint32_t addr, uint32_t data);
		uint32_t	GetReg(uint32_t addr);
		uint32_t	ReadStatus() { return status & 0x03; }
		uint32_t	ReadStatusEx() { return 0xff; }
		
		void	SetChannelMask(uint32_t mask);
	private:
		virtual void Intr(bool) {}
		
		void	SetStatus(uint32_t bit);
		void	ResetStatus(uint32_t bit);
		
		uint32_t	fnum[3];
		uint32_t	fnum3[3];
		uint8_t	fnum2[6];
		
		Channel4 ch[3];
	};

	//	YM2608(OPNA) ---------------------------------------------------
	class OPNA : public OPNABase
	{
	public:
		OPNA();
		virtual ~OPNA();
		
		bool	Init(uint32_t c, uint32_t r, const char* rhythmpath);
		bool	LoadRhythmSample(const char*);
	
		bool	SetRate(uint32_t c, uint32_t r);
		void 	Mix(int16_t* buffer, size_t nsamples);

		void	Reset();
		void 	SetReg(uint32_t addr, uint32_t data);
		uint32_t	GetReg(uint32_t addr);

		void	SetVolumeADPCM(int db);
		void	SetVolumeRhythmTotal(int db);
		void	SetVolumeRhythm(int index, int db);

		uint8_t*	GetADPCMBuffer() { return adpcmbuf; }
	private:
		struct Rhythm
		{
			uint8_t	pan;		// ぱん
			int8_t	level;		// おんりょう
			int		volume;		// おんりょうせってい
			int16_t*	sample;		// さんぷる
			uint32_t	size;		// さいず
			uint32_t	pos;		// いち
			uint32_t	step;		// すてっぷち
			uint32_t	rate;		// さんぷるのれーと
		};
	
		void	RhythmMix(int16_t* buffer, size_t count);

	// リズム音源関係
		Rhythm	rhythm[6];
		int8_t	rhythmtl;		// リズム全体の音量
		int		rhythmtvol;		
		uint8_t	rhythmkey;		// リズムのキー
	};

	//	YMF288 ---------------------------------------------------
	class Y288 : public OPNABase
	{
	public:
		Y288();
		virtual ~Y288();
		
		bool	Init(uint32_t c, uint32_t r, const char*);
		bool	LoadRhythmSample(const char*);
	
		bool	SetRate(uint32_t c, uint32_t r);
		void 	Mix(int16_t* buffer, size_t nsamples);

		void	SetVolumeRhythmTotal(int db);
		void	SetVolumeRhythm(int index, int db);

		void	Reset();
		void 	SetReg(uint32_t addr, uint32_t data);
		uint32_t	GetReg(uint32_t addr);

	private:
		struct Rhythm
		{
			uint8_t	pan;		// ぱん
			int8_t	level;		// おんりょう
			int		volume;		// おんりょうせってい
			int16_t*	sample;		// さんぷる
			uint32_t	size;		// さいず
			uint32_t	pos;		// いち
			uint32_t	step;		// すてっぷち
			uint32_t	rate;		// さんぷるのれーと
		};
	
		void	RhythmMix(int16_t* buffer, size_t count);

	// リズム音源関係
		Rhythm	rhythm[6];
		int8_t	rhythmtl;		// リズム全体の音量
		int		rhythmtvol;		
		uint8_t	rhythmkey;		// リズムのキー

		int	mode288;		// 288/2608モードフラグ
	};
}

// ---------------------------------------------------------------------------

inline void FM::OPNBase::RebuildTimeTable()
{
	int p = prescale;
	prescale = (uint8_t)-1;
	SetPrescaler(p);
}

inline void FM::OPNBase::SetVolumePSG(int db)
{
	psg.SetVolume(db);
}

#endif // FM_OPNA_H
