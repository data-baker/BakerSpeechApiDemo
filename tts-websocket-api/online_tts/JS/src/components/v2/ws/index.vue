<template>
  <div class="wx-index">
    <div style="margin-left: 20px;">
      <div><a href="https://fqihrx37dhp.feishu.cn/docx/Nw45d16hCo05bUxbqp1cx970nkd#part-ANkmdcWKPoHf9sxjpimcRI9Xn2e"
              target="_blank">TTS-V2-Websocket-文档地址</a></div>
    </div>
    <div class="page-main">
      <el-form
          ref="ruleFormRef"
          :model="ruleForm"
          :rules="rules"
          class="ruleFormRef"
          label-width="120px"
          :size="formSize"
          status-icon
      >
        <div>
          <el-form-item label="voice_name" prop="voice_name">
            <el-input v-model="ruleForm.voice_name" placeholder="请填写发音人"/>
            <p class="item-text">发音人选择, 例如：Jingjing 等等</p>
            <p class="item-text">发音人管理：开放平台【https://ai.data-baker.com/】语音合成 -> 短文本合成 -> 发音人管理 -> voice_name</p>
          </el-form-item>
          <el-form-item label="language" prop="language">
            <el-select v-model="ruleForm.language">
              <el-option label="ZH(中文和中英混)" value="ZH"/>
              <el-option label="ENG(纯英文)" value="ENG"/>
              <el-option label="YUE(粤语)" value="YUE"/>
              <el-option label="SCH(四川话)" value="SCH"/>
              <el-option label="TJH(天津话)" value="TJH"/>
              <el-option label="TAI(台湾话)" value="TAI"/>
              <el-option label="KR(韩语)" value="KR"/>
              <el-option label="BRA(巴葡语)" value="BRA"/>
              <el-option label="JP(日语)" value="JP"/>
              <el-option label="ESP(西班牙西语)" value="ESP"/>
            </el-select>
            <p class="item-text">语言选择, 例如：ZH 等等</p>
          </el-form-item>
          <el-form-item label="speed" prop="speed">
            <el-slider v-model="ruleForm.speed" :min="0" :max="10" :step="1" show-input/>
            <p class="item-text">设置播放的语速，在0～9之间（支持浮点值），默认值为5</p>
          </el-form-item>
          <el-form-item label="volume" prop="volume">
            <el-slider v-model="ruleForm.volume" :min="0" :max="10" :step="1" show-input/>
            <p class="item-text">设置语音的音量，在0～9之间（只支持整型值），默认值为5</p>
          </el-form-item>
          <el-form-item label="pitch" prop="pitch">
            <el-slider v-model="ruleForm.pitch" :min="0" :max="10" :step="1" show-input/>
            <p class="item-text">设置语音的音调，在0～9之间，（支持浮点值），默认值为5</p>
          </el-form-item>
          <el-form-item label="sample_rate" prop="sample_rate">
            <el-select v-model="ruleForm.sample_rate">
              <el-option label="8000" value="8000"/>
              <el-option label="16000" value="16000"/>
              <el-option label="24000" value="24000"/>
            </el-select>
            <p class="item-text">采样率 默认16000采样率 16000， 8000， 24000 </p>
          </el-form-item>
          <el-form-item label="timestamp" prop="timestamp" >
            <el-select v-model="ruleForm.timestamp" placeholder="请选择时间戳信息">
              <el-option label="phone" value="phone"/>
              <el-option label="word" value="word"/>
              <el-option label="both" value="both"/>
            </el-select>
          </el-form-item>

          <el-form-item>
            <p class="item-text">
              返回时间戳信息选项 <br/>
              phone 返回音子时间戳信息<br/>
              word 返回字时间戳信息<br/>
              both 同时返回音子时间戳信息和字时间戳信
            </p>
          </el-form-item>
          <el-form-item label="silence" prop="silence">
            <el-select v-model="ruleForm.silence">
              <el-option label="0" value="0"/>
              <el-option label="1" value="1"/>
              <el-option label="2" value="2"/>
            </el-select>

          </el-form-item>
          <el-form-item>
            <p class="item-text">
              设置标点符号静音时长：<br/>
              0：默认值<br/>
              1：句中标点停顿较短，适合直播、配音解说等场景<br/>
              2：句中标点停顿较长，适合朗诵、教学等场景
            </p>
          </el-form-item>

          <el-form-item label="emo_type" prop="emo_type">
            <el-input v-model="ruleForm.emo_type"/>
            <p class="item-text">
              在请求情感音色时，该参数是必填项。请求情感音色不存在的情感会报错。可选值参考 发音人列表-多情感/风格音色，如：sad
            </p>
          </el-form-item>
          <el-form-item label="emo_intensity" prop="emo_intensity">
            <el-slider v-model="ruleForm.emo_intensity" :min="1" :max="5" :step="1" show-input/>
            <p class="item-text">设置情感强度，在1～5之间（只支持整型值），不传时默认为3。</p>
          </el-form-item>
        </div>
        <div>
          <el-form-item label="text" prop="text">
            <el-input type="textarea" :rows="15" id="input_text" show-word-limit placeholder="请输入您要合成的文本"
                      maxlength="200"
                      v-model="ruleForm.text"></el-input>
          </el-form-item>
          <el-form-item>
            <el-button @click="play">{{ btnText }}</el-button>
            <el-button @click="download('pcm')">下载pcm</el-button>
            <el-button @click="download('wav')">下载wav</el-button>
          </el-form-item>
        </div>
      </el-form>
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
      btnText: "立即合成",
      ruleForm: {
        voice_name: "Jingjing", // 发音人选择, 例如：Jingjing 等等 发音人管理：开放平台【https://ai.data-baker.com/】语音合成 -> 短文本合成 -> 发音人管理 -> voice_name
        language: "ZH", // 语言选择, 例如：ZH 等等
        speed: 5, // 语速
        volume: 5, // 音量
        pitch: 5, // 音高
        sample_rate: 16000, // 采样率
        timestamp: "", // 返回时间戳信息选项phone:返回音子时间戳信息 word:返回字时间戳信息 both:同时返回音子时间戳信息和字时间戳信
        silence: 0, // 设置标点符号静音时长： 0：默认值 1：句中标点停顿较短，适合直播、配音解说等场景 2：句中标点停顿较长，适合朗诵、教学等场景
        emo_type: '', // 在请求情感音色时，该参数是必填项。请求情感音色不存在的情感会报错。可选值参考 发音人列表-多情感/风格音色，如：sad
        emo_intensity: 3, // 设置情感强度，在1～5之间（只支持整型值），不传时默认为3。
        text: '你好！标贝科技', // 合成文本
      },
      rules: {
        voice_name: [
          {required: true, message: "请填写发音人名称", trigger: "blur"},
          {min: 1, max: 30, message: "长度在1至30个字", trigger: "blur"}
        ],
        text: [
          {required: true, message: "请填写需要合成的文字", trigger: "blur"},
          {min: 1, max: 200, message: "长度在1至200个字", trigger: "blur"}
        ]

      },
      formSize: "small",
    }
  },
  mounted() {
    const that = this;
    initTts.onWillStatusChange = function (oldStatus, status) {
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
      this.$refs.ruleFormRef.validate((valid) => {
        if (valid) {
          if (initTts.status === 'init') {
            // 设置请求参数
            initTts.setParams(this.ruleForm);
            initTts.start();
          } else if (initTts.status === 'endPlay' || initTts.status === 'errorTTS') {
            initTts.start();
          } else {
            initTts.stop();
          }
        }
      })

    },
    download(val) {
      if (val === 'pcm') {
        if (initTts.rawAudioData.length) {
          downloadPCM(new Int16Array(initTts.rawAudioData))
        } else {
          alert('请先合成')
        }
      } else if (val === 'wav') {
        if (initTts.rawAudioData.length) {
          downloadWAV(new DataView(new Int16Array(initTts.rawAudioData).buffer), 16000, 16)
        } else {
          alert('请先合成')
        }
      }
    }
  },
  watch: {
    // 'ruleForm': {
    //   handler: function(newUser, oldUser) {
    //     console.log('用户信息发生变化：', newUser, oldUser);
    //   },
    //   initTts.setParams(this.ruleForm);
    ruleForm: {
      handler: function (newUser, oldUser) {
        initTts.setParams(this.ruleForm);
      },
      deep: true
    },
  }
}
</script>

<style lang="scss">
body, html {
  width: 100%;
  padding: 0;
  margin: 0;
}

.item-text {
  color: #999999;
  font-size: 12px;
  margin-top: 5px;
  margin-left: 5px;
  margin-bottom: 0;
}

.ruleFormRef {
  display: flex;
  width: 80vw;
  margin: 20px auto;

  > div:nth-child(1) {
    width: 50%;
    margin-right: 20px;
  }

  > div:nth-child(2) {
    width: 50%;
    margin-right: 20px;
  }
}
</style>