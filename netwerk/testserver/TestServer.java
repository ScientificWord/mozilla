/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

import java.io.*;
import java.net.*;

public class TestServer extends Thread {

	final static int DEFAULT_PORT = 4321;

    protected int port;
	protected ServerSocket listen_socket;

	public static void fail(Exception e, String msg) {
		System.err.println(msg + ":" + e);
		e.printStackTrace();
	}

	public TestServer() {
		this(DEFAULT_PORT);
	}

	public TestServer(int port) {
		this.port = port;
		try {
			listen_socket = new ServerSocket(port);
		}
		catch (IOException e) {
			fail(e, "Exception creating server socket");
		}
		System.out.println("Server: listening on port " + port);
		this.start();
	}

	public void run() {
		try {
			while (true) {
				Socket client_socket = listen_socket.accept();
				new Connection(client_socket);
			}
		}
		catch (IOException e) {
			fail(e, "Exception while listening");
		}
	}

	public static void main(String args[]) {
		int port = DEFAULT_PORT;
		if (args.length == 1) {
			try {
				port = Integer.parseInt(args[0]);
			}
			catch (NumberFormatException e) {
				port = DEFAULT_PORT;
			}
		}
		new TestServer(port);
	}

}

