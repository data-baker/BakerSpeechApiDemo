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
	Domain      string `json:"domain"`
	Interval    string `json:"interval,omitempty"`
	Language    string `json:"language"`
	VoiceName   string `json:"voice_name"`
	Text        string `json:"text"`
	Audiotype   string `json:"audiotype"`
	Version     string `json:"version"`
	Rate        string `json:"rate,omitempty"`
	AccessToken string `json:"access_token"`
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
	if 0 == strings.Compare("2", reqParam.Version) {
		jsonData, _ := json.Marshal(&reqParam)
		resp, err = client.Post(reqUrl, "application/json", bytes.NewReader(jsonData))
	} else {
		urls := url.Values{}
		urls.Add("text", reqParam.Text)
		urls.Add("voice_name", reqParam.VoiceName)
		urls.Add("access_token", reqParam.AccessToken)
		urls.Add("domain", "1")
		urls.Add("language", "zh")
		urls.Add("interval", reqParam.Interval)
		if strings.Contains(reqParam.VoiceName, "cc") {
			urls.Add("audiotype", "6")
			urls.Add("rate", "1")
		} else {
			if len(reqParam.Audiotype) > 0 {
				urls.Add("audiotype", reqParam.Audiotype)
			}
			if len(reqParam.Rate) > 0 {
				urls.Add("rate", reqParam.Rate)
			}
		}
		reqUrl += "?"
		reqUrl += urls.Encode()
		resp, err = client.Get(reqUrl)
	}
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
	if strings.Contains(contentType, "audio/") {
		logid := resp.Header.Get("logid")
		intervalInfo := resp.Header.Get("interval-info")
		intervalInfoX := resp.Header.Get("interval-info-x")
		fmt.Printf("interval:\t%s\ninterval_x:\t%s\n", intervalInfo, intervalInfoX)
		filename := reqParam.VoiceName + "_"
		filename += logid
		filename += "."
		filename += strings.TrimLeft(contentType, "audio/")
		fmt.Printf("output:\t\t%s\n", filename)
		_ = ioutil.WriteFile(filename, result, 0755)
		return "", nil
	}

	return string(result), fmt.Errorf("tts work failed")
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
	flag.IntVar(&audioType, "audiotype", 6, "audiotype")
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
	param := ReqTtsParams{
		AccessToken: accessToken,
		Text:        text,
		Language:    "ZH",
		Domain:      "1",
		VoiceName:   voiceName,
		Audiotype:   strconv.Itoa(audioType),
		Rate:        strconv.Itoa(rate),
	}
	if interval {
		param.Interval = "1"
	}
	body, err := SendTts("https://openapi.data-baker.com/tts", param)
	if err != nil {
		fmt.Printf("send tts http request failed. result: %s \n", body)
		return
	}
	fmt.Println("send http request success.")
}
