// 1. websocket连接：判断浏览器是否兼容，获取websocket url并连接，这里为了方便本地生成websocket url
// 2. 连接websocket，向websocket发送数据，实时接收websocket返回数据
// 3. 处理websocket返回数据为浏览器可以播放的音频数据
// 4. 播放音频数据
// ps: 该示例用到了es6中的一些语法，建议在chrome下运行

import transWorker from '@/js/transcode.worker'
import request from '@/js/request'
import {Base64} from 'js-base64'

const URL = window.URL || window.webkitURL;

// 加载并启动 worker
let workerString = transWorker.toString();
// 移除函数包裹
workerString = workerString.substr(workerString.indexOf("{") + 1);
workerString = workerString.substr(0, workerString.lastIndexOf("}"));
const workerBlob = new Blob([workerString]);
const workerURL = URL.createObjectURL(workerBlob);
const worker = new Worker(workerURL);

// 获取 client_secret,client_id 开放平台【https://ai.data-baker.com/】语音合成 -> 短文本合成 -> 授权管理
const client_secret = '';
const client_id = '';

function getWebsocketUrl() {
  return new Promise((resolve, rejected) => {
    let wsUrl = 'wss://openapi.data-baker.com/tts/wsapi';
    let tokenUrl = "https://openapi.data-baker.com/oauth/2.0/token";
    let url = `${tokenUrl}?grant_type=client_credentials&client_secret=${client_secret}&client_id=${client_id}`;
    request({
      url: url,
      method: 'get'
    }).then(res => {
      console.log(res);
      resolve({
        wsUrl: wsUrl,
        token: res.access_token
      })
    }).catch(err => {
      rejected(err)
    })
  })
}
class TTSRecorder {
  constructor({
    text = '',
    speed = '5',
    voice = '5',
    voiceName = 'Jiaojiao',
    language = 'ZH',
    pitch = '5',
    audiotype = '4'
  } = {}) {
    this.speed = speed; // 设置播放的语速，在0～9之间（支持浮点值），默认值为5
    this.voice = voice; // 设置语音的音量，在0～9之间（只支持整型值），默认值为5
    this.pitch = pitch; // 设置语音的音调，在0～9之间，（支持浮点值），默认值为5
    this.language = language; // 合成请求文本的语言 ZH(中文和中英混) ENG(纯英文，中文部分不会合成) CAT(粤语） SCH(四川话) TJH(天津话) TAI(台湾话) KR(韩语) BRA(巴葡语)
    this.text = text; // 合成文本
    this.audiotype = audiotype; // 音频种类： audiotype = 4，返回16K采样率的pcm格式，默认值 audiotype = 5，返回8K采样率的pcm格式
    this.voiceName = voiceName; // 填写支持的发音人
    this.domain = '1'; // 应用所属领域如导航、客服等，以数字进行编码，目前值固定为1
    this.audioData = [];
    this.rawAudioData = [];
    this.audioDataOffset = 0;
    this.status = 'init';
    worker.onmessage = (e) => {
      this.audioData.push(...e.data.data);
      this.rawAudioData.push(...e.data.rawAudioData)
    }
  }
  // 修改录音听写状态
  setStatus(status) {
    this.onWillStatusChange && this.onWillStatusChange(this.status, status);
    this.status = status
  }
  // 设置合成相关参数
  setParams({ speed, voice, pitch, text, voiceName }) {
    speed !== undefined && (this.speed = speed);
    voice !== undefined && (this.voice = voice);
    pitch !== undefined && (this.pitch = pitch);
    text && (this.text = text);
    voiceName && (this.voiceName = voiceName);
    this.resetAudio()
  }
  // 连接websocket
  connectWebSocket() {
    this.setStatus('ttsIng');
    return getWebsocketUrl().then(data => {
      let ttsWS;
      if ('WebSocket' in window) {
        ttsWS = new WebSocket(data.wsUrl)
      } else if ('MozWebSocket' in window) {
        // eslint-disable-next-line no-undef
        ttsWS = new MozWebSocket(data.wsUrl)
      } else {
        alert('浏览器不支持WebSocket');
        return
      }
      this.ttsWS = ttsWS;
      ttsWS.onopen = e => {
        this.webSocketSend(data.token);
        this.playTimeout = setTimeout(() => {
          this.audioPlay()
        }, 1000)
      };
      ttsWS.onmessage = e => {
        this.result(e.data)
      };
      ttsWS.onerror = e => {
        clearTimeout(this.playTimeout);
        this.setStatus('errorTTS');
        alert('WebSocket报错，请f12查看详情')
        // console.error(`详情查看：${encodeURI(url.replace('wss:', 'https:'))}`)
      };
      ttsWS.onclose = e => {
        console.log(e)
      }
    }).catch(err => {
      this.setStatus('init');
      console.log(err);
      alert('token鉴权失败，请检查client_secret，client_id 是否正确')
    })
  }
  // 处理音频数据
  transToAudioData(audioData) {}
  // websocket发送数据
  webSocketSend(token) {
    let params = {
      access_token: token,
      version: '1.0',
      tts_params: {
        speed: this.speed,
        voice: this.voice,
        pitch: this.pitch,
        language: this.language,
        text: Base64.encode(this.text),
        audiotype: this.audiotype,
        voice_name: this.voiceName,
        domain: this.domain
      }
    };
    this.ttsWS.send(JSON.stringify(params))
  }
  // websocket接收数据的处理
  result(resultData) {
    let jsonData = JSON.parse(resultData);
    console.log(jsonData);
    // 合成失败
    if (jsonData.code !== 90000) {
      alert(`合成失败: ${jsonData.code}:${jsonData.message}`);
      console.error(`${jsonData.code}:${jsonData.message}`);
      this.resetAudio();
      return
    }
    worker.postMessage(jsonData.data.audio_data);

    if (jsonData.data.end_flag === 1) {
      this.ttsWS.close()
    }
  }
  // 重置音频数据
  resetAudio() {
    this.audioStop();
    this.setStatus('init');
    this.audioDataOffset = 0;
    this.audioData = [];
    this.rawAudioData = [];
    this.ttsWS && this.ttsWS.close();
    clearTimeout(this.playTimeout)
  }
  // 音频初始化
  audioInit() {
    // web audio 各个浏览器的支持 https://blog.csdn.net/dcdjldfu334551/article/details/101308864
    let AudioContext = window.AudioContext || window.webkitAudioContext;
    if (AudioContext) {
      this.audioContext = new AudioContext();
      this.audioContext.resume();
      this.audioDataOffset = 0
    }
  }
  // 音频播放
  audioPlay() {
    this.setStatus('play');
    let audioData = this.audioData.slice(this.audioDataOffset);
    this.audioDataOffset += audioData.length;
    let audioBuffer = this.audioContext.createBuffer(1, audioData.length, 22050);
    let nowBuffering = audioBuffer.getChannelData(0);
    if (audioBuffer.copyToChannel) {
      console.log(new Float32Array(audioData));
      audioBuffer.copyToChannel(new Float32Array(audioData), 0, 0)
    } else {
      for (let i = 0; i < audioData.length; i++) {
        nowBuffering[i] = audioData[i]
      }
    }
    let bufferSource = this.bufferSource = this.audioContext.createBufferSource();
    bufferSource.buffer = audioBuffer;
    console.log(audioBuffer);
    bufferSource.connect(this.audioContext.destination);
    bufferSource.start();
    bufferSource.onended = event => {
      if (this.status !== 'play') {
        return
      }
      if (this.audioDataOffset < this.audioData.length) {
        this.audioPlay()
      } else {
        this.audioStop()
      }
    }
  }
  // 音频播放结束
  audioStop() {
    this.setStatus('endPlay');
    clearTimeout(this.playTimeout);
    this.audioDataOffset = 0;
    if (this.bufferSource) {
      try {
        this.bufferSource.stop()
      } catch (e) {
        console.log(e)
      }
    }
  }
  start() {
    if(this.audioData.length) {
      this.audioPlay()
    } else {
      if (!this.audioContext) {
        this.audioInit()
      }
      if (!this.audioContext) {
        alert('该浏览器不支持webAudioApi相关接口');
        return
      }
      this.connectWebSocket()
    }
  }
  stop() {
    this.setStatus('endPlay');
    this.audioStop()
  }
}
export default TTSRecorder;
