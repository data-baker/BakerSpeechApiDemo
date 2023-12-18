<template>
  <div class="wx-index">
    <div style="margin-left: 20px;">
      <div><a href="https://fqihrx37dhp.feishu.cn/docx/Nw45d16hCo05bUxbqp1cx970nkd#part-ANkmdcWKPoHf9sxjpimcRI9Xn2e" target="_blank">TTS-Websocket-文档地址</a></div>
    </div>
    <div class="page-main">
      <div>
        <label for="input_text"></label><textarea id="input_text" placeholder="请输入您要合成的文本" maxlength="300" v-model="text"></textarea>
      </div>
      <div>
        <button @click="play">{{ btnText }}</button>
        <button @click="download('pcm')">下载pcm</button>
        <button @click="download('wav')">下载wav</button>
      </div>
    </div>
  </div>
</template>

<script>
import TTSRecorder from './js/ws'
let initTts = new TTSRecorder();
import {downloadPCM, downloadWAV} from "@/js/download";

export default {
  name: "wsIndex",
  data() {
    return {
      text: '你好！标贝科技',
      btnText: "立即合成"
    }
  },
  mounted() {
    const that = this;
    initTts.onWillStatusChange = function(oldStatus, status) {
      let btnText = {
        init: '立即合成',
        ttsIng: '正在合成',
        play: '停止播放',
        endPlay: '重新播放',
        errorTTS: '合成失败',
      };
      that.btnText = btnText[status];
    }
  },
  methods: {
    play() {
      console.log(initTts.status);
      if (initTts.status === 'init') {
        // 设置请求参数
        initTts.setParams({
          text: this.text
        });
        initTts.start();
      } else if (initTts.status === 'endPlay' || initTts.status === 'errorTTS') {
        initTts.start();
      } else {
        initTts.stop();
      }
    },
    download(val) {
      if (val === 'pcm') {
        if (initTts.rawAudioData.length){
          downloadPCM(new Int16Array(initTts.rawAudioData))
        } else {
          alert('请先合成')
        }
      } else if (val === 'wav') {
        if (initTts.rawAudioData.length){
          downloadWAV(new DataView(new Int16Array(initTts.rawAudioData).buffer), 16000, 16)
        } else {
          alert('请先合成')
        }
      }
    }
  },
  watch: {
    'text': function () {
      initTts.setStatus('init');
      // 设置请求参数
      initTts.setParams({
        text: this.text
      });
    }
  }
}
</script>

<style scoped>
body,html {
  width: 100%;
  padding: 0;
  margin: 0;
}
input:focus, textarea:focus {
  outline: none;
}
button {
  width: 110px;
  height: 35px;
  border: none;
  background: rgb(7, 96, 235);
  color: #ffffff;
  line-height: 3px;
  margin-right: 15px;
  margin-top: 15px;
  border-radius: 5px;
  cursor: pointer;
}
.page-main {
  width: 80%;
  margin: 50px auto;
}
#input_text {
  width: 80%;
  height: 300px;
  border: 1px rgba(7, 96, 235, 0.78) solid;
  border-radius: 5px;
  padding: 5px;
}
</style>