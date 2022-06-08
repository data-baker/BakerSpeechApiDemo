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
	"strconv"
	"strings"
	"time"
)

type AuthInfo struct {
	AccessToken string `json:"access_token"`
	ExpiresIn   int    `json:"expires_in"`
	Scope       string `json:"scope"`
}

type ReqTtsParams struct {
	Domain    string `json:"domain"`
	Interval  string `json:"interval,omitempty"`
	Language  string `json:"language"`
	VoiceName string `json:"voice_name"`
	Text      string `json:"text"`
	Audiotype string `json:"audiotype,omitempty"`
	Rate      string `json:"rate,omitempty"`
}
type WsReqParam struct {
	AccessToken string       `json:"access_token"`
	Version     string       `json:"version"`
	TtsParams   ReqTtsParams `json:"tts_params"`
}

type WsMsgData struct {
	Idx       int    `json:"idx"`
	AudioData string `json:"audio_data"`
	AudioType string `json:"audio_type"`
	Interval  string `json:"interval"`
	IntervalX string `json:"interval_x"`
	EndFlag   int    `json:"end_flag"`
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

func SendTts(urlStr string, reqParam WsReqParam) error {
	con, _, err := websocket.DefaultDialer.Dial(urlStr, nil)
	if err != nil {
		fmt.Printf("websocket dial failed. url: %s\n", urlStr)
		return err
	}
	defer con.Close()
	if strings.Contains(reqParam.TtsParams.VoiceName, "cc") {
		reqParam.TtsParams.Audiotype = "5"
	}
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

	filename := reqParam.TtsParams.VoiceName + "_"
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
		if msg.Code != 90000 {
			return fmt.Errorf("error_code %d, msg: %s", msg.Code, msg.Message)
		}
		fmt.Printf("index:\t%d\n", msg.Data.Idx)
		fmt.Printf("interval:\t%s\n", msg.Data.Interval)
		fmt.Printf("interval_x:\t%s\n", msg.Data.IntervalX)
		fmt.Printf("endflag:\t%d\n\n", msg.Data.EndFlag)
		if msg.Data.EndFlag == 1 {
			return nil
		}
		audioData, err := base64.StdEncoding.DecodeString(msg.Data.AudioData)
		if err != nil {
			return err
		}
		_, _ = writer.Write(audioData)
		_ = writer.Flush()
	}
	return nil
}

func main() {
	var (
		clientId, clientSecret, text, voiceName string
		audioType, rate                         int
		interval                                bool
	)
	flag.StringVar(&clientId, "cid", "", "client id")
	flag.StringVar(&clientSecret, "cs", "", "client secret")
	flag.StringVar(&text, "t", "标贝科技", "合成文本")
	flag.StringVar(&voiceName, "v", "jingjing", "发音人")
	flag.IntVar(&audioType, "audiotype", 4, "audiotype")
	flag.IntVar(&rate, "rate", 0, "rate")
	flag.BoolVar(&interval, "interval", false, "interval")
	flag.Parse()
	if len(os.Args) < 2 {
		flag.Usage()
		return
	}
	if len(clientId) <= 0 || len(clientSecret) <= 0 || len(text) <= 0 || len(voiceName) <= 0 {
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
	base64Text := base64.StdEncoding.EncodeToString([]byte(text))
	param := WsReqParam{
		AccessToken: accessToken,
		Version:     "1.0",
		TtsParams: ReqTtsParams{
			Text:      base64Text,
			Language:  "ZH",
			Domain:    "1",
			VoiceName: voiceName,
			Audiotype: strconv.Itoa(audioType),
			Rate:      strconv.Itoa(rate),
		},
	}
	if interval {
		param.TtsParams.Interval = "1"
	}
	err = SendTts("wss://openapi.data-baker.com/tts/wsapi", param)
	if err != nil {
		fmt.Printf("send tts websocket request failed. err: %s \n", err.Error())
		return
	}
	fmt.Println("send websocket request success.")
}
