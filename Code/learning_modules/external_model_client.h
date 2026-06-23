/* Kom8ndor IEEE 802.11bn Simulator
 *
 * Copyright (c) 2026, Universitat Pompeu Fabra.
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 *
 * -----------------------------------------------------------------
 * Author  : Francesc Wilhelmi
 * -----------------------------------------------------------------
 */

/**
 * external_model_client.h: POSIX Unix-socket client for an external ML server
 *
 * Used by LearningAlgorithm when learning_mechanism == LEARNING_MECHANISM_EXTERNAL.
 * The Python server (ml_server_*.py) must be started before the simulation.
 *
 * Wire protocol (both sides native-endian, assumes same host):
 *   C++ → Python : uint32_t n_feat, then n_feat * 4 bytes (float[])
 *   Python → C++ : n_out  * 4 bytes (float[])
 *
 * C++98 compatible — no lambdas, no auto, no in-class member initializers.
 */

#ifndef _EXTERNAL_MODEL_CLIENT_H_
#define _EXTERNAL_MODEL_CLIENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/un.h>
#include <unistd.h>

struct ExternalModelClient {

	int sockfd;

	ExternalModelClient() : sockfd(-1) {}

	/* Open connection to the Unix-domain socket at socket_path.
	 * Aborts if the server is not listening. */
	void Connect(const char* socket_path) {
		struct sockaddr_un addr;
		sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
		if (sockfd < 0) {
			perror("[ExternalModelClient] socket()");
			exit(EXIT_FAILURE);
		}
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
		/* 'connect' is a COST keyword — call via syscall to avoid the conflict */
		if (syscall(SYS_connect, sockfd, (struct sockaddr*)&addr,
		            (int)sizeof(addr)) < 0) {
			perror("[ExternalModelClient] connect");
			fprintf(stderr,
				"[ExternalModelClient] Is ml_server_*.py running on %s?\n",
				socket_path);
			exit(EXIT_FAILURE);
		}
		printf("[ExternalModelClient] Connected to %s\n", socket_path);
	}

	/* Send features to the server; receive n_out floats back.
	 * Uses retry loops to handle short send/recv on a local socket. */
	void Query(float* features, int n_feat, float* out, int n_out) {
		/* --- send header (4 bytes: number of features) --- */
		uint32_t header = (uint32_t) n_feat;
		if (SendAll(&header, (int)sizeof(uint32_t)) < 0) {
			fprintf(stderr, "[ExternalModelClient] send header failed\n");
			exit(EXIT_FAILURE);
		}
		/* --- send feature vector --- */
		if (SendAll(features, n_feat * (int)sizeof(float)) < 0) {
			fprintf(stderr, "[ExternalModelClient] send features failed\n");
			exit(EXIT_FAILURE);
		}
		/* --- receive result --- */
		if (RecvAll(out, n_out * (int)sizeof(float)) < 0) {
			fprintf(stderr, "[ExternalModelClient] recv result failed\n");
			exit(EXIT_FAILURE);
		}
	}

	/* Close and invalidate the socket. Safe to call more than once.
	 * Sends n_feat=0 sentinel so the Python server exits cleanly without
	 * relying on EOF detection (which is unreliable on some platforms). */
	void Close() {
		if (sockfd >= 0) {
			uint32_t sentinel = 0;
			SendAll(&sentinel, (int)sizeof(uint32_t));
			shutdown(sockfd, SHUT_RDWR);
			syscall(SYS_close, sockfd);  /* 'close' may also conflict with COST */
			sockfd = -1;
		}
	}
private:

	/* Retry send until all bytes are written; returns 0 on success, -1 on error. */
	int SendAll(void* buf, int len) {
		int sent = 0;
		while (sent < len) {
			ssize_t s = send(sockfd, (char*)buf + sent, (size_t)(len - sent), 0);
			if (s <= 0) return -1;
			sent += (int)s;
		}
		return 0;
	}

	/* Retry recv until all bytes are read; returns 0 on success, -1 on error. */
	int RecvAll(void* buf, int len) {
		int received = 0;
		while (received < len) {
			ssize_t r = recv(sockfd, (char*)buf + received, (size_t)(len - received), 0);
			if (r <= 0) return -1;
			received += (int)r;
		}
		return 0;
	}

};

#endif /* _EXTERNAL_MODEL_CLIENT_H_ */
