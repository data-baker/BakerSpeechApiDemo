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
type ReqTtsParams struct {
	AccessToken string `json:"access_token"`
	Text        string `json:"text"`
	VoiceName   string `json:"voiceName"`
	NotifyUrl   string `json:"notifyUrl"`
	Interval    int    `json:"interval,omitempty"`
	Speed       string `json:"speed,omitempty"`
	Volume      string `json:"volume,omitempty"`
	AudioType   int    `json:"audiotype"`
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

func SendTts(URL string, reqParam ReqTtsParams) (string, error) {
	// 超时时间：60秒
	client := &http.Client{Timeout: 60 * time.Second}
	reqUrl := URL
	fmt.Printf("reqUrl: %s\n\n", reqUrl)
	var resp *http.Response
	var err error
	jsonData, _ := json.Marshal(&reqParam)
	resp, err = client.Post(reqUrl, "application/json", bytes.NewReader(jsonData))
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
	return string(result), nil
}

func main() {
	var (
		clientId, clientSecret, text, voiceName, notifyUrl, speed, volume string
		audioType                                                         int
		interval                                                          bool
	)
	flag.StringVar(&clientId, "cid", "", "client id")
	flag.StringVar(&clientSecret, "cs", "", "client secret")
	flag.StringVar(&notifyUrl, "notify_url", "", "接收合成后的结果url")
	flag.StringVar(&text, "t", "标贝科技", "合成文本")
	flag.StringVar(&voiceName, "v", "jingjing", "发音人")
	flag.IntVar(&audioType, "audiotype", 6, "audiotype")
	flag.BoolVar(&interval, "interval", false, "interval")
	flag.StringVar(&speed, "speed", "", "合成speed")
	flag.StringVar(&volume, "volume", "", "合成volume")
	flag.Parse()
	if len(os.Args) < 2 {
		flag.Usage()
		return
	}
	if len(clientId) <= 0 || len(clientSecret) <= 0 || len(text) <= 0 || len(voiceName) <= 0 || len(notifyUrl) <= 0 {
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
	param := ReqTtsParams{
		AccessToken: accessToken,
		Text:        text,
		VoiceName:   voiceName,
		AudioType:   audioType,
		NotifyUrl:   notifyUrl,
		Speed:       speed,
		Volume:      volume,
	}
	if interval {
		param.Interval = 1
	}
	body, err := SendTts("https://openapi.data-baker.com/asynctts/synthesis/work", param)
	if err != nil {
		fmt.Printf("send tts http request failed. result: %s \n", body)
		return
	}
	fmt.Printf("send http request success. result: %s \n", body)
}
