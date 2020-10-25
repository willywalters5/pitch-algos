import numpy as np
from scipy.io.wavfile import read, write
from matplotlib import pyplot as plt
from pitch_parselmouth import compute_pitch_praat
from Testing import compute_errors,compute_errors_mean

frame_dur=512
zero_crossing_threshold=frame_dur/5
min_pitch=60
max_pitch=300
fft_value=512

def calculate_pitch_CEP(wav_file,frame_dur):
    print("Current wav file: {} ".format(wav_file))
    rate,data=read(wav_file)
    frame_num=int(len(data)/frame_dur)+1
    frames=np.zeros((frame_num,frame_dur))
    pitch=np.zeros((frame_num))
    for i in range(frame_num):
        if i==frame_num-1:
            last_frame_len=(len(data)-frame_dur*i)
            frames[i,:last_frame_len]=data[frame_dur*i:len(data)]
        else:
            frames[i,:]=data[frame_dur*i:frame_dur*(i+1)]
    #silence detector <1/15 of maximum peak absolute signal value within the utterance, then it's called silence
    signal_peak=np.amax(np.abs(data))
    for i in range(frame_num):
        curr_peak=np.amax(np.abs(frames[i,:]))
        if curr_peak<signal_peak/12: continue
        zero_crossings_num = len(np.where(np.diff(np.signbit(frames[i,:])))[0])
        if zero_crossings_num>=zero_crossing_threshold: continue
        #compute cepstrum
        windowed=np.hamming(frame_dur)*frames[i]
        cep=np.fft.ifft(np.log(np.abs(np.fft.fft(windowed,n=frame_dur))))
        min_interval,max_interval=rate//max_pitch,rate//min_pitch
        pitch_interval=np.argmax(np.square(cep[min_interval:max_interval]))+min_interval
        pitch[i]=rate/pitch_interval
        # print(i,pitch[i],pitch_interval,min_interval,max_interval)
        # plt.plot(np.abs(cep[1:]))
        # plt.show()
        #search range 60-300 Hz
    pitch_praat=compute_pitch_praat(wav_file,pitch,True)
    # print("CEP",pitch,len(pitch))
    # print("praat",pitch_praat,len(pitch_praat))
    total_len=min(len(pitch),len(pitch_praat))
    compute_errors(pitch[:total_len],pitch_praat[:total_len],rate,True)
    return pitch[:total_len],pitch_praat[:total_len]

pitches=[]
pitches_praat=[]
for i in range(1,9):
    pitch,pitch_praat=calculate_pitch_CEP("Recordings/{}_AM1.wav".format(i),frame_dur)
    pitches.append(pitch)
    pitches_praat.append(pitch_praat)
    pitch,pitch_praat=calculate_pitch_CEP("Recordings/{}_AM2.wav".format(i),frame_dur)
    pitches.append(pitch)
    pitches_praat.append(pitch_praat)
    pitch,pitch_praat=calculate_pitch_CEP("Recordings/{}_AF1.wav".format(i),frame_dur)
    pitches.append(pitch)
    pitches_praat.append(pitch_praat)
compute_errors_mean(pitches,pitches_praat,12000)
