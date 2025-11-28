import sys, numpy as np, pandas as pd

FS=1000.0; N=400
TAU_LP=1.8; TAU_DC=10.0; TAU_AMP=0.6
USE_HANN=True

def lockin_block(seg, use_hann=True):
    seg = seg.astype(float); seg = seg - seg.mean()
    n = np.arange(len(seg), dtype=float)
    if use_hann and len(seg)>1:
        w = 0.5*(1-np.cos(2*np.pi*n/(len(seg)-1))); seg = seg*w
    ph = 2*np.pi*n/len(seg)
    re = float(np.dot(seg, np.cos(ph))); im = float(np.dot(seg, np.sin(ph)))
    amp = (2.0/len(seg))*np.hypot(re, im)
    return re, im, amp

def series_lockin(sig, blocks, dt_block, tau_lp, tau_dc, tau_amp, use_hann):
    i_lp=q_lp=i_dc=q_dc=amp_ema=0.0
    a_lp = 1-np.exp(-dt_block/max(tau_lp,1e-9))
    a_dc = 1-np.exp(-dt_block/max(tau_dc,1e-9))
    a_am = 1-np.exp(-dt_block/max(tau_amp,1e-9))
    out=[]
    for i0,i1 in blocks:
        re, im, _ = lockin_block(sig[i0:i1], use_hann)
        i_lp += a_lp*(re - i_lp); q_lp += a_lp*(im - q_lp)
        i_dc += a_dc*(i_lp - i_dc); q_dc += a_dc*(q_lp - q_dc)
        i_hp = i_lp - i_dc; q_hp = q_lp - q_dc
        amp_hp = (2.0/N)*np.hypot(i_hp,q_hp)
        amp_ema = amp_ema + a_am*(amp_hp - amp_ema)
        out.append(amp_ema)
    return np.array(out)

def stats(a): 
    m=float(np.mean(a)); s=float(np.std(a)); cv = s/m*100 if m else float('nan')
    return m,s,cv

def main(xlsx):
    df = pd.read_excel(xlsx)
    # colonne attese
    for c in ["Sample","10_27","12_40","11_30","3_33","Digital"]:
        if c not in df.columns: raise SystemExit(f"Colonna mancante: {c}")
    S10 = df["10_27"].to_numpy(float)
    S12 = df["12_40"].to_numpy(float)
    S11 = df["11_30"].to_numpy(float)
    Srf = df["3_33"].to_numpy(float)
    dig = df["Digital"].to_numpy(float)

    # allinea ai fronti di salita di Digital
    thr=32.0
    rising = np.where((np.roll(dig<thr,1) & (dig>=thr)))[0]
    rising = rising[rising>0]
    start = int(rising[0]) if len(rising)>0 else 0

    # blocchi fissi da 400
    num_blocks = (len(Srf)-start)//N
    blocks = [(start+b*N, start+(b+1)*N) for b in range(num_blocks)]
    dt_block = N/FS

    A10 = series_lockin(S10, blocks, dt_block, TAU_LP, TAU_DC, TAU_AMP, USE_HANN)
    A11 = series_lockin(S11, blocks, dt_block, TAU_LP, TAU_DC, TAU_AMP, USE_HANN)
    A12 = series_lockin(S12, blocks, dt_block, TAU_LP, TAU_DC, TAU_AMP, USE_HANN)
    Arf = series_lockin(Srf, blocks, dt_block, TAU_LP, TAU_DC, TAU_AMP, USE_HANN)

    eps=1e-18
    T10=(A10+eps)/(Arf+eps); T11=(A11+eps)/(Arf+eps); T12=(A12+eps)/(Arf+eps)
    dA10=-np.log(T10); dA11=-np.log(T11); dA12=-np.log(T12)

    for name, arr in [("S10",A10),("S11",A11),("S12",A12),("Sref",Arf),
                      ("T10",T10),("T11",T11),("T12",T12),
                      ("dA10",dA10),("dA11",dA11),("dA12",dA12)]:
        m,s,cv = stats(arr)
        print(f"{name}: mean={m:.6g}, std={s:.6g}, CV={cv:.3f}% (blocks={len(arr)})")

    out = pd.DataFrame({
        "t_s": np.arange(num_blocks)*dt_block,
        "S10":A10,"S11":A11,"S12":A12,"Sref":Arf,
        "T10":T10,"T11":T11,"T12":T12,
        "dA10":dA10,"dA11":dA11,"dA12":dA12
    })
    out.to_csv("lockin_fixed_block_amps.csv", index=False)
    print("Salvato: lockin_fixed_block_amps.csv")

if __name__ == "__main__":
    main(sys.argv[1])
