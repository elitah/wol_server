package main

import (
	"encoding/json"
	"fmt"
	"net"
	"net/url"
	"notice/httpreq"
	"os"
	"os/signal"
	"syscall"
	"time"
)

func main() {
	// 创建系统信号chan
	sigChan := make(chan os.Signal, 1)

	// 注册chan
	signal.Notify(sigChan)

	option := parseArgs()

	if nil == option {
		os.Exit(0)
	}

	fmt.Println(option)

	if addr, err := net.ResolveUDPAddr("udp", fmt.Sprintf(option.listenAddr)); nil == err {
		if conn, err := net.ListenUDP("udp", addr); nil == err {
			go func() {
				timemap := make(map[string]time.Time)
				var buffer [1024]byte
				for {
					if n, err := conn.Read(buffer[:]); nil == err {
						timenow := time.Now()
						cmd := &struct {
							Cmd string `json:"cmd"`
							Key string `json:"key"`
						}{}
						if err := json.Unmarshal(buffer[:n], cmd); nil == err {
							switch cmd.Cmd {
							case "send_notice":
								last, _ := timemap[cmd.Key]
								if last.IsZero() || last.Add(time.Minute).Before(timenow) {
									timemap[cmd.Key] = timenow
									u := url.URL{}
									u.Scheme = "https"
									u.Host = option.domain
									u.Path = "/" + option.token + ".send"
									q := u.Query()
									q.Set("text", "报警信息，门被打开了")
									q.Set("desp", "报警信息，门被打开了，时间"+time.Now().Format("2006-01-02 15:04:05"))
									u.RawQuery = q.Encode()
									httpreq.HttpRequestGet(u.String())
								}
							}
						} else {
							fmt.Println(err)
						}
					} else {
						fmt.Println(err)

						time.Sleep(3 * time.Second)

						os.Exit(0)

						break
					}
				}
			}()

			// 主循环
		DONE:
			for {
				select {
				case sig := <-sigChan:
					fmt.Println("SIG:", sig)
					switch sig {
					case syscall.SIGHUP, syscall.SIGINT, syscall.SIGQUIT, syscall.SIGTERM:
						// 防死锁3秒后强行退出
						go func() {
							time.Sleep(3 * time.Second)
							os.Exit(0)
						}()
						break DONE
					}
				case <-time.After(time.Second):
				}
			}
		} else {
			fmt.Println(err)
		}
	} else {
		fmt.Println(err)
	}
}
