# Radio 
## 1. Device communication

No one knows

## 2. Reading data

**type**: <span style="color:green">uint8_t </span><br>
First processing , change char type (**uint8_t**) to **int8** or **double** _(better double)_<br>
_**[ToDo]:** how to find f0? (power spectral density)_

## 3. Converting to complex

Device return data as a stream of sequentially IQ data like _I1, Q1, I2, Q2 ... etc_. <br>
Convert data to complex value like Zn = In + i\*Qn.

## 4. Shifting to baseband

complex_signal \* exp(-i\*w)<br>
_w =  2PI\*fc/fs\*t_ - as vector of discrete values with dt step <br>
Euler's formul: exp(i*w) = cos(w) + i\*sin(w) 

\+ filtering IIR: <br>
| var  |  0 |  1 |  2 |  3 | 4 |
|---|---|---|---|---|---|
| b | 0.0004  |  0.0015  |  0.0022  |  0.0015  |  0.0004 |
| a |  1.0000 | -3.2070  |  3.9205  | -2.1578  |  0.4502 |

## 5. Downsampling/decimation

After filtering we discard unnecessary samples by decimation wih factor _**fs/bwSERV**_<br>
fs - sample rate <br>
bwSERV - bandwitch of FM service (**256e3**Hz)

## 6. FM demodulation

C(n) = C(n) \* C\*(n-1)  <--- conjugate

demodulated(n) = atan2(C(n).Im / C(n).Re) <br>
_atan2 - Compute arc tangent with two parameters_

Then, decimate signal with factor of bwSERV/bwAUDIO after filtering<br>
b (*e-5):

    0.0020    0.0162    0.0567    0.1134    0.1417    0.1134    0.0567    0.0162    0.0020

a:

    1.0000   -7.2117   22.9518  -42.0901   48.6330  -36.2476   17.0156   -4.5989    0.5479

## 7. Mono
Now we have mono singal  <3 <br>
Normalization is optional - ym = ym /(max(abs(ym))+0.001)  

## 8. PLL

Pilot signal in FM modulation is 19kHz signal around 38kHz. This freq and second harmonic give us left and right channel. <br>
PLL is used to find pilot phase.

## 9. Stereo

![alt text](https://upload.wikimedia.org/wikipedia/commons/thumb/c/cd/RDS_vs_DirectBand_FM-spectrum2.svg/1920px-RDS_vs_DirectBand_FM-spectrum2.svg.png "Logo Title Text 1")


 



