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
type ReqTtsParams struct {
	AccessToken string
	AudioFormat string
	SampleRate  int
	domain      string
	hotwordId   string
	diylmId     string
	Channel     int
}

type Response struct {
	Code    int    `json:"code"`
	Taskid  string `json:"taskid"`
	TraceId string `json:"trace_id"`
}

type RecognitionText struct {
	LeftResult  string `json:"left_result"`
	RightResult string `json:"right_result"`
}

type RecognitionResult struct {
	Code    int             `json:"code"`
	Taskid  string          `json:"taskid"`
	TraceId string          `json:"trace_id"`
	Info    string          `json:"info"`
	Text    RecognitionText `json:"text"`
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

func CreateTask(reqUrl, file string, reqParam ReqTtsParams) (string, error) {
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
	if len(reqParam.hotwordId) > 0 {
		req.Header.Add("hotwordid", reqParam.hotwordId)
	}
	if len(reqParam.diylmId) > 0 {
		req.Header.Add("diylmid", reqParam.diylmId)
	}
	if reqParam.Channel > 0 {
		req.Header.Add("channel", strconv.Itoa(reqParam.Channel))
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
		return "", err
	}
	if response.Code != 20000 {
		return string(result), errors.New("request failed code != 20000")
	}

	return response.Taskid, nil
}

func QueryResult(reqUrl, taskId, access_token string) (int, error) {
	client := &http.Client{Timeout: 60 * time.Second}
	req, err := http.NewRequest(http.MethodPost, reqUrl, nil)
	if err != nil {
		return 0, err
	}
	req.Header.Add("access_token", access_token)
	req.Header.Add("taskid", taskId)
	resp, err := client.Do(req)
	if err != nil {
		return 0, err
	}
	defer resp.Body.Close()
	if resp.StatusCode != 200 {
		fmt.Printf("send http request return status code != 200\n")
		return 0, errors.New("status code is not 200")
	}
	resultJson, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		fmt.Printf("readall http body failed, err: %s \n", err)
		return 0, err
	}
	result := RecognitionResult{}
	if err = json.Unmarshal(resultJson, &result); err != nil {
		return 0, err
	}
	if 20000 != result.Code {
		return result.Code, fmt.Errorf("code:%d, traceId:%s info:%s", result.Code, result.TraceId, result.Info)
	}
	fmt.Printf("code:\t\t%d\ntraceId:\t%s\nleft_result:\t%s\nright_result:\t%s\n",
		result.Code, result.TraceId, result.Text.LeftResult, result.Text.RightResult)
	return result.Code, nil
}

func main() {
	var (
		clientId, clientSecret, file, audio_format, domain string
		sample_rate, channel                               int
	)
	flag.StringVar(&clientId, "cid", "", "client id")
	flag.StringVar(&clientSecret, "cs", "", "client secret")
	flag.StringVar(&file, "f", "", "识别文件")
	flag.IntVar(&sample_rate, "sample_rate", 16000, "音频采样率")
	flag.IntVar(&channel, "channel", 1, "录音文件声道数")
	flag.StringVar(&domain, "domain", "", "模型名称")
	flag.StringVar(&audio_format, "audio_format", "wav", "音频格式")
	flag.Parse()
	if len(os.Args) < 2 {
		flag.Usage()
		return
	}
	if len(clientId) <= 0 || len(clientSecret) <= 0 || len(file) <= 0 || len(audio_format) <= 0 {
		fmt.Println("parameter error!!!")
		return
	}
	// 获取令牌
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
		SampleRate:  sample_rate,
		AudioFormat: audio_format,
		Channel:     channel,
		domain:      domain,
	}
	// 创建识别任务
	result, err := CreateTask("https://openapi.data-baker.com/asr/starttask?", file, param)
	if err != nil {
		fmt.Printf("send request failed. result: %s \n", result)
		return
	}
	fmt.Printf("create task success. traceId: %s\n", result)
	for {
		// 查询识别结果
		code, err := QueryResult("https://openapi.data-baker.com/asr/taskresult?", result, accessToken)
		if err == nil {
			break
		}
		if code != 20001 && code != 20005 {
			fmt.Printf("query result failed. error: %s\n", err.Error())
			break
		}
		fmt.Printf("query result failed. %s\n", err.Error())
		time.Sleep(time.Second)
	}
	fmt.Println("request finish...")
}
