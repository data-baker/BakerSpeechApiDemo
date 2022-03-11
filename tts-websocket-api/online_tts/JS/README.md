# 在线合成H5 Websocket版 Demo
# 标贝科技-TTS-Websocket-使用说明

### 本地运行
1.在本目录cmd中打开,以下命令
```
npm install
npm run serve
```
2.在浏览器中打开 http://localhost:8080/

3.在src -> js -> ws.js 配置client_secret,client_id 获取（开放平台【https://ai.data-baker.com/）语音合成 -> 在线合成 -> 授权管理]

4.API开发文档地址 【https://www.data-baker.com/specs/file/tts_api_websocket】

5. 入口文件在APP.vue 
```$xslt
// 初始化 WebSocket 文件方法
import TTSRecorder from './js/ws'
let initTts = new TTSRecorder();

// 播放
<button @click="play">{{ btnText }}</button>
// 下载为文件
<button @click="download('pcm')">下载pcm</button>
<button @click="download('wav')">下载wav</button>
```

