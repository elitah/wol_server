package main

import (
	"flag"
	"fmt"
)

type Options struct {
	listenAddr string
	domain     string
	token      string
}

func parseArgs() *Options {
	listenAddr := flag.String("listenAddr", ":50174", "listenAddr")
	domain := flag.String("domain", "", "domain")
	token := flag.String("token", "", "Your token")

	flag.Parse()

	if "" != *listenAddr && "" != *domain && "" != *token && 54 == len(*token) {
		return &Options{
			listenAddr: *listenAddr,
			domain:     *domain,
			token:      *token,
		}
	}

	flag.Usage()

	return nil
}

func (this *Options) String() string {
	str := fmt.Sprintln("---------------------------------")

	str += fmt.Sprintln("listenAddr:", this.listenAddr)
	str += fmt.Sprintln("domain:", this.domain)
	str += fmt.Sprintln("token:", this.token)

	str += fmt.Sprintln("---------------------------------------------------------")

	return str
}
