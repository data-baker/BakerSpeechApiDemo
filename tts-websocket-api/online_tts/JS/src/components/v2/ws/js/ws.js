// 1. websocket连接：判断浏览器是否兼容，获取websocket url并连接，这里为了方便本地生成websocket url
// 2. 连接websocket，向websocket发送数据，实时接收websocket返回数据
// 3. 处理websocket返回数据为浏览器可以播放的音频数据
// 4. 播放音频数据
// ps: 该示例用到了es6中的一些语法，建议在chrome下运行

import transWorker from '@/js/transcode.worker'
import request from '@/js/request'

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
  onWillStatusChange; // 状态改变回调
  constructor({
    text = '',
    speed = '5',
    voice = '5',
    voice_name = '',
    pitch = '5',
    language = "ZH", // 语言选择, 例如：ZH 等等
    sample_rate = 16000, // 采样率
    timestamp = "", // 返回时间戳信息选项phone:返回音子时间戳信息 word:返回字时间戳信息 both:同时返回音子时间戳信息和字时间戳信
    silence = 0, // 设置标点符号静音时长： 0：默认值 1：句中标点停顿较短，适合直播、配音解说等场景 2：句中标点停顿较长，适合朗诵、教学等场景
    emo_type = '', // 在请求情感音色时，该参数是必填项。请求情感音色不存在的情感会报错。可选值参考 发音人列表-多情感/风格音色，如：sad
    emo_intensity = 3, // 设置情感强度，在1～5之间（只支持整型值），不传时默认为3。
  } = {}) {
    this.speed = speed; // 设置播放的语速，在0～9之间（支持浮点值），默认值为5
    this.voice = voice; // 设置语音的音量，在0～9之间（只支持整型值），默认值为5
    this.pitch = pitch; // 设置语音的音调，在0～9之间，（支持浮点值），默认值为5
    this.language = language; // 合成请求文本的语言 ZH(中文和中英混) ENG(纯英文，中文部分不会合成) CAT(粤语） SCH(四川话) TJH(天津话) TAI(台湾话) KR(韩语) BRA(巴葡语)
    this.text = text; // 合成文本
    this.voice_name = voice_name; // 填写支持的发音人
    this.sample_rate = sample_rate;
    this.timestamp = timestamp;
    this.silence = silence;
    this.emo_type = emo_type;
    this.emo_intensity = emo_intensity;
    this.domain = '1'; // 应用所属领域如导航、客服等，以数字进行编码，目前值固定为1
    this.audioData = [];
    this.rawAudioData = [];
    this.audioDataOffset = 0;
    this.status = 'init';
    worker.onmessage = (e) => {
      this.audioData = [...this.audioData, ...e.data.data]; // 合并二进制数据
      this.rawAudioData = [...this.rawAudioData, ...e.data.rawAudioData];
    }
  }
  // 修改录音听写状态
  setStatus(status) {
    this.onWillStatusChange && this.onWillStatusChange(this.status, status);
    this.status = status
  }

  /**
   * 设置合成相关参数
   * @param data
   */
  setParams(data) {
    this.resetAudio()
    this.speed = data.speed; // 设置播放的语速，在0～9之间（支持浮点值），默认值为5
    this.voice = data.voice; // 设置语音的音量，在0～9之间（只支持整型值），默认值为5
    this.pitch = data.pitch; // 设置语音的音调，在0～9之间，（支持浮点值），默认值为5
    this.language = data.language; // 合成请求文本的语言 ZH(中文和中英混) ENG(纯英文，中文部分不会合成) CAT(粤语） SCH(四川话) TJH(天津话) TAI(台湾话) KR(韩语) BRA(巴葡语)
    this.text = data.text; // 合成文本
    this.voice_name = data.voice_name; // 填写支持的发音人
    this.sample_rate = data.sample_rate;
    this.timestamp = data.timestamp;
    this.silence = data.silence;
    this.emo_type = data.emo_type;
    this.emo_intensity = data.emo_intensity;
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
  // websocket发送数据
  webSocketSend(token) {
    let params = {
      access_token: token,
      version: '2.1',
      tts_params: {
        speed: this.speed,
        voice: this.voice,
        pitch: this.pitch,
        language: this.language,
        text: this.text,
        voice_name: this.voice_name,
        domain: this.domain,
        sample_rate: this.sample_rate,
        timestamp: this.timestamp,
        silence: this.silence,
        emo_type: this.emo_type,
        emo_intensity: this.emo_intensity,
      }
    };
    this.ttsWS.send(JSON.stringify(params))
  }
  // websocket接收数据的处理
  result(resultData) {
    let jsonData = JSON.parse(resultData);
    console.log(jsonData);
    // 合成失败
    if (jsonData.err_no !== 0) {
      alert(`合成失败: ${jsonData.err_no}:${jsonData.err_msg}`);
      console.error(`${jsonData.err_no}:${jsonData.err_msg}`);
      this.resetAudio();
      return
    }
    worker.postMessage(jsonData.result.audio_data);

    if (jsonData.result.end_flag === 1) {
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
      console.log(this.audioContext)
      this.audioContext.resume();
      this.audioDataOffset = 0
    }
  }
  // 音频播放
  audioPlay() {
    this.setStatus('play');
    let audioData = this.audioData.slice(this.audioDataOffset); // 截取从audioDataOffset开始之后的数据
    this.audioDataOffset += audioData.length; // 计算偏移量
    // 获取采样点
    const sampleRateMap = {
      "8000": 10550,
      "16000": 22050,
      "24000": 30170
    }; // 采样点

    let sample_rate_number = sampleRateMap[this.sample_rate.toString()] || 22050; // 采样点
    let audioBuffer = this.audioContext.createBuffer(1, audioData.length, sample_rate_number); // 22050创建单声道的缓存文件，采样点为音频数据的长度
    let nowBuffering = audioBuffer.getChannelData(0); // 只处理单声道
    if (audioBuffer.copyToChannel) {
      audioBuffer.copyToChannel(new Float32Array(audioData), 0, 0) // 从audioData复制数据到audioBuffer
    } else {
      for (let i = 0; i < audioData.length; i++) {
        nowBuffering[i] = audioData[i] //
      }
    }
    let bufferSource = this.bufferSource = this.audioContext.createBufferSource();
    bufferSource.buffer = audioBuffer;
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
