/////////////////////////////////////////////////////////////////////////////
// Name:        handle.hpp
// Purpose:     Abstraction to handle commands/messages
// Author:      jay-tux
// Created:     July 12, 2022 11:03 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_SERVER_HANDLE_HPP
#define DOTCHAT_SERVER_HANDLE_HPP

#include "tls/tls_bytestream.hpp"
#include "protocol/message.hpp"

/**
 * \short Namespace for all code related to the server.
 */
namespace dotchat::server {
/**
 * \short Reads a message from the byte stream, then chooses the correct handler and returns a response.
 * \param in The stream to read from.
 * \returns The response message.
 */
proto::message handle(tls::bytestream &in);
}

#endif //DOTCHAT_SERVER_HANDLE_HPP
