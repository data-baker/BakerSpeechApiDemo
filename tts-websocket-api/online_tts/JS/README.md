# 标贝科技-TTS-使用说明

### 本地运行
1.在本目录cmd中打开,以下命令
```
npm install
npm run serve
```
2.在浏览器中打开 http://localhost:8080/


4. 体验前需要配置client_secret && client_id
```$xslt

获取： client_secret,client_id 开放平台【https://ai.data-baker.com/】语音合成 -> 短文本合成 -> 授权管理

v1: 
    * 配置client_secret && client_id --> 
    * 路径 src/components/v1/ws/js/ws.js 
    
v2: 
    webSocket: 
        * 配置client_secret && client_id 
        * 路径 src/components/v2/ws/js/ws.js
        
    http:
        * 配置client_secret && client_id
        * 路径 src/components/v2/RESTful/index.vue
```

