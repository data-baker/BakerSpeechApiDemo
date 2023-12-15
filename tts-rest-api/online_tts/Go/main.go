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
	"strings"
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
	Domain     int    `json:"domain"`
	Language   string `json:"language"`
	VoiceName  string `json:"voice_name"`
	Text       string `json:"text"`
	AudioFmt   string `json:"audio_fmt"`
	SampleRate int    `json:"sample_rate,omitempty"`
	Timestamp  string `json:"timestamp,omitempty"`
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

func SendTts(urlStr string, reqParam ReqParam) (string, error) {
	// 超时时间：60秒
	client := &http.Client{Timeout: 60 * time.Second}
	reqUrl := urlStr
	fmt.Printf("reqUrl: %s\n\n", reqUrl)
	jsonData, _ := json.Marshal(&reqParam)
	resp, err := client.Post(reqUrl, "application/json", bytes.NewReader(jsonData))
	if err != nil {
		fmt.Printf("send http tts request failed, err: %s \n", err)
		return "", err
	}
	defer resp.Body.Close()
	if resp.StatusCode != 200 {
		fmt.Printf("send http tts request return status code != 200\n")
		return "", errors.New("status code is not 200")
	}
	result, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		fmt.Printf("readall http body failed, err: %s \n", err)
		return "", err
	}
	contentType := resp.Header.Get("Content-Type")
	fmt.Println(contentType)
	if strings.Contains(contentType, "audio/") {
		filename := reqParam.TtsParams.VoiceName
		filename += "."
		filename += strings.TrimLeft(contentType, "audio/")
		if 0 == strings.Compare(reqParam.TtsParams.Timestamp, "both") || 0 == strings.Compare(reqParam.TtsParams.
			Timestamp, "phone") || 0 == strings.Compare(reqParam.TtsParams.Timestamp, "word") {
			length := (int(result[0]) << 24) + (int(result[1]) << 16) + (int(result[2]) << 8) + int(result[3])
			if length > len(result) {
				return "", errors.New("response data length error")
			}
			jsonData := result[4 : length+4]
			audioData := result[length+4:]
			fmt.Printf("timestamp json string:\t%s\n", jsonData)
			_ = ioutil.WriteFile(filename, audioData, 0755)
		} else {
			_ = ioutil.WriteFile(filename, result, 0755)
		}
		fmt.Printf("output:\t%s\n", filename)
		return "", nil
	}
	return string(result), fmt.Errorf("tts work failed")
}

func main() {
	var (
		clientId, clientSecret, text, voiceName, audioFmt, timeStamp string
		sampleRate                                                   int
	)
	flag.StringVar(&clientId, "cid", "", "client id")
	flag.StringVar(&clientSecret, "cs", "", "client secret")
	flag.StringVar(&text, "t", "标贝科技", "合成文本, 默认: 标贝科技")
	flag.StringVar(&voiceName, "v", "jingjing", "发音人, 默认Jingjing")
	flag.StringVar(&audioFmt, "af", "mp3", "audio格式. 默认mp3")
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
	param := ReqParam{
		AccessToken: accessToken,
		Version:     "2.1",
		TtsParams: ReqTtsParams{
			Domain:     1,
			Language:   "ZH",
			VoiceName:  voiceName,
			Text:       text,
			AudioFmt:   audioFmt,
			SampleRate: sampleRate,
			Timestamp:  timeStamp,
		},
	}
	body, err := SendTts("https://openapi.data-baker.com/tts", param)
	if err != nil {
		fmt.Printf("send tts http request failed. err:%s \n result: %s \n", err.Error(), body)
		return
	}
	fmt.Println("send http request success.")
}
