package httpreq

import (
	"bytes"
	"crypto/tls"
	"crypto/x509"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"net"
	"net/http"
	"net/url"
	"runtime"
	"time"
)

var (
	tlsConfig  *tls.Config   = nil
	timeoutMax time.Duration = 10 * time.Second
)

func newHttpDial(netw, addr string) (net.Conn, error) {
	cnt := 0
	t := time.Now()
	for {
		if elapse := time.Since(t); timeoutMax > elapse {
			remain := timeoutMax - elapse
			if 500*time.Millisecond > remain {
				remain = 500 * time.Millisecond
			} else if 3*time.Second < remain {
				remain = 3 * time.Second
			}
			cnt++
			c, err := net.DialTimeout(netw, addr, remain)
			if nil == err {
				c.SetDeadline(time.Now().Add(timeoutMax))
				return c, nil
			} else {
				if e, ok := err.(*url.Error); ok {
					if !e.Timeout() {
						return nil, e.Err
					}
				} else if e, ok := err.(*net.OpError); ok {
					if !e.Timeout() {
						return nil, e.Err
					}
					if !e.Temporary() {
						return nil, e.Err
					}
				} else {
					return nil, err
				}
			}
		} else {
			break
		}
	}
	return nil, errors.New(fmt.Sprintf("Request Timeout(cnt: %v, %v)", cnt, time.Since(t)))
}

func newHttpClient() *http.Client {
	return &http.Client{
		Transport: &http.Transport{
			Dial:            newHttpDial,
			TLSClientConfig: tlsConfig,
		},
	}
}

func HttpCAUpdate(data []byte) error {
	if "arm" == runtime.GOARCH {
		pool := x509.NewCertPool()
		if pool.AppendCertsFromPEM(data) {
			tlsConfig = &tls.Config{
				RootCAs:            pool,
				InsecureSkipVerify: false,
			}
			return nil
		}
		return errors.New("Unable to append CA certs")
	}
	return nil
}

func HttpSetTimeout(sec int) {
	if 1 <= sec {
		timeoutMax = time.Duration(sec) * time.Second
	}
}

func HttpRequestGet(request_url string) ([]byte, error) {
	if "" != request_url {
		if c := newHttpClient(); nil != c {
			resp, err := c.Get(request_url)
			if nil == err {
				defer resp.Body.Close()
				return ioutil.ReadAll(resp.Body)
			}
			return nil, err
		}
	}
	return nil, errors.New("Unable to create http client")
}

func HttpRequestGetJson(request_url string, response_data interface{}) error {
	if result, err := HttpRequestGet(request_url); nil == err {
		response := &struct {
			Status   bool        `json:"status"`
			Response interface{} `json:"response"`
		}{
			Response: response_data,
		}
		if err := json.Unmarshal(result, response); nil == err {
			if response.Status {
				return nil
			} else {
				return errors.New("server response false")
			}
		} else {
			return err
		}
	} else {
		return err
	}
}

func HttpRequestPost(request_url string, data []byte) ([]byte, error) {
	if "" != request_url {
		if c := newHttpClient(); nil != c {
			resp, err := c.Post(request_url, "application/json; encoding=utf-8", bytes.NewReader(data))
			if nil == err {
				defer resp.Body.Close()
				return ioutil.ReadAll(resp.Body)
			}
			return nil, err
		}
	}
	return nil, errors.New("Unable to create http client")
}

func HttpRequestPostJson(request_url string, request_data, response_data interface{}) error {
	if data, err := json.Marshal(request_data); nil == err {
		if result, err := HttpRequestPost(request_url, data); nil == err {
			response := &struct {
				Status   bool        `json:"status"`
				Response interface{} `json:"response"`
			}{
				Response: response_data,
			}
			if err := json.Unmarshal(result, response); nil == err {
				if response.Status {
					return nil
				} else {
					return errors.New("server response false")
				}
			} else {
				return err
			}
		} else {
			return err
		}
	} else {
		return err
	}
}
