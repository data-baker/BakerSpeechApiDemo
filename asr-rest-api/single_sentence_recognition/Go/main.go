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
	"time"
)

type AuthInfo struct {
	AccessToken string `json:"access_token"`
	ExpiresIn   int    `json:"expires_in"`
	Scope       string `json:"scope"`
}
type ReqParams struct {
	AccessToken string
	AudioFormat string
	SampleRate  int
	domain      string
	addPct      string
	hotwordId   string
	diylmId     string
}
type Response struct {
	Code int    `json:"code"`
	Text string `json:"text"`
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

func SendAsr(reqUrl, file string, reqParam ReqParams) (string, error) {
	// 超时时间：60秒
	client := &http.Client{Timeout: 60 * time.Second}
	fmt.Printf("reqUrl: %s\n\n", reqUrl)
	fileContent, err := ioutil.ReadFile(file)
	if err != nil {
		return "", err
	}
	req, err := http.NewRequest(http.MethodPost, reqUrl, bytes.NewReader(fileContent))
	if err != nil {
		return "", err
	}
	req.Header.Add("access_token", reqParam.AccessToken)
	req.Header.Add("audio_format", reqParam.AudioFormat)
	req.Header.Add("sample_rate", strconv.Itoa(reqParam.SampleRate))
	if len(reqParam.domain) > 0 {
		req.Header.Add("domain", reqParam.domain)
	}
	if len(reqParam.addPct) > 0 {
		req.Header.Add("add_pct", reqParam.addPct)
	}
	if len(reqParam.hotwordId) > 0 {
		req.Header.Add("hotwordid", reqParam.hotwordId)
	}
	if len(reqParam.diylmId) > 0 {
		req.Header.Add("diylmid", reqParam.diylmId)
	}
	resp, err := client.Do(req)

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
	response := Response{}
	if err = json.Unmarshal(result, &response); err != nil {
		return string(result), err
	}
	if 20000 == response.Code {
		return response.Text, nil
	}
	return string(result), nil
}

func main() {
	var (
		clientId, clientSecret, file string
	)
	param := ReqParams{}
	flag.StringVar(&clientId, "cid", "", "client id")
	flag.StringVar(&clientSecret, "cs", "", "client secret")
	flag.IntVar(&param.SampleRate, "sample_rate", 16000, "sample_rate: 8000, 16000")
	flag.StringVar(&param.domain, "domain", "common", "domain")
	flag.StringVar(&param.AudioFormat, "audio_format", "wav", "音频编码格式 wav, pcm")
	flag.StringVar(&param.addPct, "add_pct", "true", "add_pct true: 加标点，默认值. false：不添加标点")
	flag.StringVar(&param.hotwordId, "hotwordid", "", "配置的热词组的id")
	flag.StringVar(&param.diylmId, "diylmid", "", "asr个性化模型的id")
	flag.StringVar(&file, "f", "", "需要识别的音频文件")
	flag.Parse()
	if len(os.Args) < 2 {
		flag.Usage()
		return
	}
	if len(clientId) <= 0 || len(clientSecret) <= 0 || len(file) <= 0 {
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
	param.AccessToken = accessToken
	result, err := SendAsr("https://openapi.data-baker.com/asr/api?", file, param)
	if err != nil {
		fmt.Printf("send http request failed. result: %s \n", result)
		return
	}
	fmt.Printf("send http request success. result: %s\n", result)
}
