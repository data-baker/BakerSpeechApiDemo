package main

import (
	"bytes"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
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

type ReqParam struct {
	Version     string       `json:"version"`
	AccessToken string       `json:"access_token"`
	TtsParams   ReqTtsParams `json:"tts_params"`
}

type ReqTtsParams struct {
	Language   string `json:"language"`
	VoiceName  string `json:"voice_name"`
	Text       string `json:"text"`
	AudioFmt   string `json:"audio_fmt"`
	SampleRate int    `json:"sample_rate,omitempty"`
	Timestamp  int    `json:"timestamp,omitempty"`
	NotifyUrl  string `json:"notify_url,omitempty"`
}
type CommonResponse struct {
	ErrNo  int         `json:"err_no"`
	ErrMsg string      `json:"err_msg"`
	LogId  string      `json:"log_id"`
	Result interface{} `json:"result"`
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

func SendHttp(urlStr string, reqParam interface{}) (string, error) {
	// 超时时间：60秒
	client := &http.Client{Timeout: 60 * time.Second}
	fmt.Printf("reqUrl: %s\n\n", urlStr)
	var resp *http.Response
	var err error
	if nil != reqParam {
		jsonData, _ := json.Marshal(&reqParam)
		resp, err = client.Post(urlStr, "application/json", bytes.NewReader(jsonData))
	} else {
		resp, err = client.Get(urlStr)
	}
	if err != nil {
		fmt.Printf("send http request failed, err: %s \n", err)
		return "", err
	}
	defer resp.Body.Close()
	if resp.StatusCode != 200 {
		fmt.Printf("send http request return status code != 200\n")
		return "", errors.New("status code is not 200")
	}
	result, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		fmt.Printf("readall http body failed, err: %s \n", err)
		return "", err
	}
	return string(result), nil
}

func main() {
	var (
		clientId, clientSecret, text, voiceName, notifyUrl, audioFmt string
		sampleRate, timeStamp                                        int
	)
	flag.StringVar(&clientId, "cid", "", "client id")
	flag.StringVar(&clientSecret, "cs", "", "client secret")
	flag.StringVar(&notifyUrl, "notify_url", "", "接收合成后的结果url")
	flag.StringVar(&text, "t", "标贝科技", "合成文本")
	flag.StringVar(&voiceName, "v", "jingjing", "发音人")
	flag.StringVar(&audioFmt, "af", "MP3", "音频格式默认MP3")
	flag.IntVar(&sampleRate, "sr", 16000, "音频采样率，默认16000")
	flag.IntVar(&timeStamp, "timestamp", 0, "时间戳功能, 默认不开启")
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
	param := ReqParam{
		AccessToken: accessToken,
		Version:     "2.1",
		TtsParams: ReqTtsParams{
			Language:   "ZH",
			VoiceName:  voiceName,
			Text:       text,
			AudioFmt:   audioFmt,
			SampleRate: sampleRate,
			Timestamp:  timeStamp,
			NotifyUrl:  notifyUrl,
		},
	}
	body, err := SendHttp("https://openapi.data-baker.com/asynctts/synthesis/work", param)
	if err != nil {
		fmt.Printf("send tts http request failed. err:%s \n result: %s \n", err.Error(), body)
		return
	}
	fmt.Printf("send http request success. result: %s \n", body)
	res := CommonResponse{}
	_ = json.Unmarshal([]byte(body), &res)
	if 0 != res.ErrNo {
		fmt.Printf("request failed!!! \n")
		return
	}
	// 开始轮训合成结果
	tmpMap := res.Result.(map[string]interface{})
	workId := tmpMap["work_id"]
	urlTplStr := "https://openapi.data-baker.com/asynctts/synthesis/query?client_id=%s&access_token=%s&work_id=%s&version=2.1"
	urlStr := fmt.Sprintf(urlTplStr, clientId, accessToken, workId)
	for {
		fmt.Println("query result")
		body, err = SendHttp(urlStr, nil)
		if err != nil {
			break
		}
		resWork := CommonResponse{}
		_ = json.Unmarshal([]byte(body), &resWork)
		if 0 == resWork.ErrNo {
			fmt.Printf("work result: %s \n\n", body)
			break
		}
		if 21040001 != resWork.ErrNo {
			fmt.Printf("work failed. response: %s \n\n", body)
			break
		}
		time.Sleep(time.Second * 5)
	}
}
