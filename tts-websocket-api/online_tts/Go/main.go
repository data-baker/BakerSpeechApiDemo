package main

import (
	"bufio"
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

type ReqTtsParams struct {
	Domain     int    `json:"domain"`
	Language   string `json:"language"`
	VoiceName  string `json:"voice_name"`
	Text       string `json:"text"`
	AudioFmt   string `json:"audio_fmt,omitempty"`
	SampleRate int    `json:"sample_rate,omitempty"`
	Timestamp  string `json:"timestamp,omitempty"`
}
type WsReqParam struct {
	AccessToken string       `json:"access_token"`
	Version     string       `json:"version"`
	TtsParams   ReqTtsParams `json:"tts_params"`
}

type WsMsgData struct {
	Idx       int         `json:"idx"`
	AudioData string      `json:"audio_data"`
	AudioType string      `json:"audio_type"`
	EndFlag   int         `json:"end_flag"`
	Timestamp interface{} `json:"timestamp"`
}
type WsMsg struct {
	ErrNo  int       `json:"err_no"`
	ErrMsg string    `json:"err_msg"`
	LogId  string    `json:"log_id"`
	Result WsMsgData `json:"result"`
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

func SendTts(urlStr string, reqParam WsReqParam) error {
	con, _, err := websocket.DefaultDialer.Dial(urlStr, nil)
	if err != nil {
		fmt.Printf("websocket dial failed. url: %s\n", urlStr)
		return err
	}
	defer con.Close()
	err = con.WriteJSON(reqParam)
	if err != nil {
		fmt.Printf("websocket write json failed. url: %s\n", urlStr)
		return err
	}
	var f *os.File
	defer func() {
		if f != nil {
			_ = f.Close()
		}
	}()

	filename := reqParam.TtsParams.VoiceName
	filename += ".pcm"
	f, err = os.Create(filename)
	if err != nil {
		return err
	}
	fmt.Printf("output:\t%s\n", filename)
	writer := bufio.NewWriter(f)
	for {
		msg := WsMsg{}
		err = con.ReadJSON(&msg)
		if err != nil {
			fmt.Printf("websocket read json failed. url: %s\n", urlStr)
			return err
		}
		if 0 != msg.ErrNo {
			return fmt.Errorf("error_code %d, msg: %s", msg.ErrNo, msg.ErrMsg)
		}
		fmt.Printf("index:\t%d\n", msg.Result.Idx)
		fmt.Printf("endflag:\t%d\n", msg.Result.EndFlag)
		fmt.Printf("timestamp:\t%v\n\n", msg.Result.Timestamp)
		if len(msg.Result.AudioData) > 0 {
			audioData, err := base64.StdEncoding.DecodeString(msg.Result.AudioData)
			if err != nil {
				return err
			}
			_, _ = writer.Write(audioData)
			_ = writer.Flush()
		}
		if msg.Result.EndFlag == 1 {
			break
		}
	}
	return nil
}

func main() {
	var (
		clientId, clientSecret, text, voiceName, timeStamp string
		sampleRate                                         int
	)
	flag.StringVar(&clientId, "cid", "", "client id")
	flag.StringVar(&clientSecret, "cs", "", "client secret")
	flag.StringVar(&text, "t", "标贝科技", "合成文本")
	flag.StringVar(&voiceName, "v", "jingjing", "发音人")
	flag.IntVar(&sampleRate, "sr", 16000, "采样率. 默认16000")
	flag.StringVar(&timeStamp, "timestamp", "", "时间戳参数")
	flag.Parse()
	if len(os.Args) < 2 {
		flag.Usage()
		return
	}
	if len(clientId) <= 0 || len(clientSecret) <= 0 || len(text) <= 0 || len(voiceName) <= 0 {
		fmt.Println("parameter error!!!")
		return
	}
	accessToken, err := GetToken("https://openapi.data-baker.com/oauth/2.0/token", clientId, clientSecret)
	if err != nil {
		fmt.Println("get access token failed!!!! please check your client_id and client_secret.")
		return
	}
	param := WsReqParam{
		AccessToken: accessToken,
		Version:     "2.1",
		TtsParams: ReqTtsParams{
			Text:       text,
			VoiceName:  voiceName,
			Domain:     1,
			Language:   "ZH",
			AudioFmt:   "PCM",
			SampleRate: sampleRate,
			Timestamp:  timeStamp,
		},
	}
	err = SendTts("wss://openapi.data-baker.com/tts/wsapi", param)
	if err != nil {
		fmt.Printf("send tts websocket request failed. err: %s \n", err.Error())
		return
	}
	fmt.Println("send websocket request success.")
}
