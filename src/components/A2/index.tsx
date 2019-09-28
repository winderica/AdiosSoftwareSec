import { Button, Checkbox, FormControl, FormControlLabel, Input, InputAdornment, InputLabel, MenuItem, TextField, Typography } from '@material-ui/core';
import { useSnackbar } from 'notistack';
import React, { ChangeEvent, useCallback, useState } from 'react';
import small from '../../samples/A2.json';
import large from '../../samples/R3.json';
import { useStyles } from '../../styles/style';
import { getTypes } from '../../utils/getTypes';
import { Expansion } from '../Expansion';

export const A2 = () => {
    const { enqueueSnackbar } = useSnackbar();
    const [similarities, setSimilarities] = useState<number[]>([]);
    const classes = useStyles();
    const [code1, setCode1] = useState(0);
    const [isLarge, setIsLarge] = useState(false);
    const [types, setTypes] = useState('size_t,subprocess_start_fn,FILE');
    const [isSending, setIsSending] = useState(false);

    const sendRequest = useCallback(async () => {
        try {
            if (isSending) return;
            setIsSending(true);
            const result = await fetch(
                `http://127.0.0.1:3001`,
                {
                    method: 'POST',
                    body: JSON.stringify({
                        code1: (isLarge ? large : small)[code1],
                        code2: (isLarge ? large : small)[0],
                        types: getTypes(types)
                    }),
                    headers: {
                        'Content-Type': 'application/json'
                    }
                },
            );
            const json = await result.json();
            if (json.value) {
                setSimilarities([json.value]);
            } else {
                enqueueSnackbar(json.error, { variant: 'error' });
            }
            setIsSending(false);
        } catch ({ message }) {
            setIsSending(false);
            enqueueSnackbar(message, { variant: 'error' });
        }
    }, [isSending, code1, types]);

    const run = () => {
        try {
            const codes = isLarge ? large : small;
            console.time('Multithread' + codes.length);
            const result = window.main.multithread(codes, codes.map(() => codes[code1]), getTypes(types));
            if (result.includes(-1)) {
                enqueueSnackbar('Error occurred in parsing', { variant: 'error' });
            } else {
                setSimilarities(result);
            }
            console.timeEnd('Multithread' + codes.length);
        } catch ({ message }) {
            enqueueSnackbar(message, { variant: 'error' });
        }
    };

    const setCode = (event: ChangeEvent<{ name?: string; value: unknown }>) => {
        setCode1(event.target.value as number);
        setSimilarities([]);
    };

    const handleInput = (event: ChangeEvent<HTMLInputElement>) => {
        setTypes(event.target.value);
    };

    const handleCheck = () => {
        setIsLarge((prev) => !prev);
    };

    return (
        <div>
            <Expansion code1={(isLarge ? large : small)[code1]} />
            <div className={classes.itemContainer}>
                <TextField
                    select
                    label='code1'
                    value={code1}
                    onChange={setCode}
                >
                    {(isLarge ? large : small).map((i, j) => <MenuItem key={j} value={j}>{j}</MenuItem>)}
                </TextField>
                <TextField
                    label='types'
                    value={types}
                    onChange={handleInput}
                />
                <FormControlLabel
                    control={<Checkbox checked={isLarge} onChange={handleCheck} color='primary' />}
                    label='Switch to Large Sample'
                />
                <Button onClick={run}>Multithreading</Button>
                <Button onClick={sendRequest}>Send Request </Button>
            </div>
            <div className={classes.resultContainer}>
                <Typography variant='caption' display='block'>Notification:</Typography>
                <Typography variant='caption' display='block'>Server should be launched when sending request</Typography>
                <Typography variant='caption' display='block'>Large samples may take a lot of time depending on performance of CPU</Typography>
                {similarities.map((i, j) => <div key={j}>
                    <FormControl>
                        <InputLabel htmlFor='similarity'>{`Similarity of ${j} and ${code1}`}</InputLabel>
                        <Input
                            id='similarity'
                            value={i * 100}
                            endAdornment={<InputAdornment position='end'>%</InputAdornment>}
                        />
                    </FormControl>
                </div>)}
            </div>
        </div>
    );
};
