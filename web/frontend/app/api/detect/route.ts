import { NextRequest, NextResponse } from 'next/server';
import React from 'react'

interface ExtendedRequestInit extends RequestInit {
  duplex?: 'half';
}

export const config = {
    api: {
        bodyParser: false,
    },
};

interface Result {
    ndet: number,
    image: string,
    labelUrl: string,
}

export async function POST(req: NextRequest) {
    const backendUrl = 'http://localhost:3000/detect';
    const contentType = req.headers.get('content-type');

    if (!contentType) {
        return NextResponse.json({ error: 'Missing content-type' }, { status: 400 });
    }

    try{
        const response = await fetch(backendUrl, {
            method: 'POST',
            headers: {
                'Content-Type': contentType,
            },
            body: req.body,
            duplex: 'half'
        } as ExtendedRequestInit);

        if(!response.ok){
            return NextResponse.json({ error: 'Backend error' }, { status: response.status });
        }

        const data: Result = await response.json();
        return NextResponse.json({
            ndet: data.ndet,
            image: data.image,
        }, {status: 200});
    }catch(error){
        console.log(error);
        return NextResponse.json({ error: 'Internal server error' }, { status: 500 });
    }
}