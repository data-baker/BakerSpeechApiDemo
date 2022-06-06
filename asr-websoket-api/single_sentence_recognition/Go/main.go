package main

import (
	"encoding/base64"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"github.com/gorilla/websocket"
	"io/ioutil"
	"net/http"
	"net/url"
	"os"
	"time"
)

type AuthInfo struct {
	AccessToken string `json:"access_token"`
	ExpiresIn   int    `json:"expires_in"`
	Scope       string `json:"scope"`
}

type ReqParams struct {
	AudioData       string `json:"audio_data"`
	AudioFormat     string `json:"audio_format"`
	SampleRate      int    `json:"sample_rate"`
	ReqIdx          int    `json:"req_idx"`
	Domain          string `json:"domain,omitempty"`
	AddPct          bool   `json:"add_pct"`
	Hotwordid       string `json:"hotwordid,omitempty"`
	Diylmid         string `json:"diylmid,omitempty"`
	EnableVad       bool   `json:"enable_vad"`
	MaxBeginSilence int    `json:"max_begin_silence,omitempty"`
	MaxEndSilence   int    `json:"max_end_silence,omitempty"`
}
type WsReqParam struct {
	AccessToken string    `json:"access_token"`
	Version     string    `json:"version"`
	Params      ReqParams `json:"asr_params"`
}

type WsMsgData struct {
	ResIdx    int      `json:"res_idx"`
	Nbest     []string `json:"nbest"`
	Uncertain []string `json:"uncertain"`
	EndFlag   int      `json:"end_flag"`
}
type WsMsg struct {
	Code    int       `json:"code"`
	Message string    `json:"message"`
	TraceId string    `json:"trace_id"`
	Data    WsMsgData `json:"data"`
}

const grantType string = "client_credentials"

func GetToken(reqUrl, clientId, clientSecret string) (string, error) {
	// 超时时间：60秒
	client := &http.Client{Timeout: 60 * time.Second}
	urlParams := url.Values{}
	urlParams.Add("grant_type", grantType)
	urlParams.Add("client_id", clientId)
	urlParams.Add("client_secret", clientSecret)
	httpUrl := reqUrl
	httpUrl += "?"
	httpUrl += urlParams.Encode()
	fmt.Printf("httpUrl: %s\n\n", httpUrl)
	resp, err := client.Get(httpUrl)
	if err != nil {
		fmt.Printf("send http get token request failed, err: %s\n", err)
		return "", err
	}
	defer resp.Body.Close()
	result, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		fmt.Printf("http get token request, readall body failed, err: %s\n", err)
		return "", err
	}
	authInfo := AuthInfo{}
	err = json.Unmarshal(result, &authInfo)
	if err != nil {
		fmt.Printf("auth info json.Unmarshal err: %s \n", err)
		return "", err
	}
	if len(authInfo.AccessToken) <= 0 {
		return "", errors.New("access token is null")
	}
	fmt.Printf("get token success. info: %#v \n", authInfo)
	return authInfo.AccessToken, nil
}

func RecvMsg(conn *websocket.Conn, chn chan error) {
	var err error
	for {
		msg := WsMsg{}
		err = conn.ReadJSON(&msg)
		if err != nil {
			fmt.Printf("websocket read json failed.\n")
			break
		}
		if msg.Code != 90000 {
			err = fmt.Errorf("error_code %d, msg: %s traceId: %s ", msg.Code, msg.Message, msg.TraceId)
			break
		}
		fmt.Printf("ResIdx:\t%d\nnbest:\t%v\n", msg.Data.ResIdx, msg.Data.Nbest)
		if len(msg.Data.Uncertain) > 0 {
			fmt.Printf("uncertain:\t%v\n", msg.Data.Uncertain)
		}
		fmt.Printf("endflag:\t%d\n\n", msg.Data.EndFlag)
		if msg.Data.EndFlag == 1 {
			break
		}
	}
	chn <- err
}

func SendAsr(urlStr, file string, reqParam WsReqParam) error {
	conn, _, err := websocket.DefaultDialer.Dial(urlStr, nil)
	if err != nil {
		fmt.Printf("websocket dial failed. url: %s\n", urlStr)
		return err
	}
	defer conn.Close()

	audioData, err := ioutil.ReadFile(file)
	if err != nil {
		fmt.Println("read file failed!!!")
		return err
	}
	// channel 等待返回使用
	chn := make(chan error)
	// 开启接收websocket 消息 协程
	go RecvMsg(conn, chn)

	for index := 0; index < len(audioData); index += 5120 {
		reqIdx := index / 5120
		if index+5120 >= len(audioData) {
			reqIdx = -reqIdx
			audio := audioData[index:]
			reqParam.Params.AudioData = base64.StdEncoding.EncodeToString(audio)
		} else {
			audio := audioData[index : index+5120]
			reqParam.Params.AudioData = base64.StdEncoding.EncodeToString(audio)
		}
		reqParam.Params.ReqIdx = reqIdx
		fmt.Printf("send req_idx: %d\n", reqIdx)
		err = conn.WriteJSON(reqParam)
		if err != nil {
			fmt.Printf("websocket write json failed. url: %s\n", urlStr)
			chn <- err
			break
		}
	}
	fmt.Println("send audio data finish.")
	// 等待channel 返回
	err = <-chn
	return err
}

func main() {
	var (
		clientId, clientSecret, file, AudioFormat, domain, hotwordid, diylmid string
		SampleRate, maxBeginSilence, maxEndSilence                            int
		addPct, enableVad                                                     bool
	)
	flag.StringVar(&clientId, "cid", "", "client id")
	flag.StringVar(&clientSecret, "cs", "", "client secret")
	flag.StringVar(&file, "f", "", "识别音频文件")
	flag.StringVar(&AudioFormat, "audio_format", "wav", "音频编码格式wav, pcm")
	flag.IntVar(&SampleRate, "sample_rate", 16000, "音频采样率8000, 16000")
	flag.StringVar(&domain, "domain", "", "模型名称")
	flag.BoolVar(&addPct, "add_pct", true, "加标点")
	flag.StringVar(&hotwordid, "hotwordid", "", "配置的热词组的id")
	flag.StringVar(&diylmid, "diylmid", "", "asr个性化模型的id")
	flag.BoolVar(&enableVad, "enable_vad", false, "静音检测")
	flag.Parse()
	if len(os.Args) < 2 {
		flag.Usage()
		return
	}
	if len(clientId) <= 0 || len(clientSecret) <= 0 || len(file) <= 0 || len(AudioFormat) <= 0 {
		fmt.Println("parameter error!!!")
		return
	}
	accessToken, err := GetToken("https://openapi.data-baker.com/oauth/2.0/token",
		clientId,
		clientSecret,
	)
	if err != nil {
		fmt.Println("get access token failed!!!! please check your client_id and client_secret.")
		return
	}
	param := WsReqParam{
		AccessToken: accessToken,
		Version:     "1.0",
		Params: ReqParams{
			AudioFormat:     AudioFormat,
			SampleRate:      SampleRate,
			ReqIdx:          0,
			Domain:          domain,
			AddPct:          addPct,
			Hotwordid:       hotwordid,
			Diylmid:         diylmid,
			EnableVad:       enableVad,
			MaxBeginSilence: maxBeginSilence,
			MaxEndSilence:   maxEndSilence,
		},
	}
	err = SendAsr("wss://openapi.data-baker.com/asr/wsapi", file, param)
	if err != nil {
		fmt.Printf("send websocket request failed. err: %s \n", err.Error())
		return
	}
	fmt.Println("send websocket request success.")
}
