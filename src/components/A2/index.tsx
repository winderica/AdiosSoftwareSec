import {
    Button,
    Divider,
    ExpansionPanel,
    ExpansionPanelDetails,
    ExpansionPanelSummary,
    FormControl,
    Input,
    InputAdornment,
    InputLabel,
    MenuItem,
    TextField,
    Typography
} from '@material-ui/core';
import { useSnackbar } from 'notistack';
import React, { ChangeEvent, useCallback, useState } from 'react';
import codes from '../../samples/R3.json';
import { useStyles } from '../../styles/style';
import { getTypes } from '../../utils/getTypes';
import { CodeHighlight } from '../Highlight';

export const A2 = () => {
    const { enqueueSnackbar } = useSnackbar();
    const [similarities, setSimilarities] = useState<number[]>([]);
    const classes = useStyles();
    const [code1, setCode1] = useState(0);
    const [types, setTypes] = useState('');
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
                        code1: codes[code1],
                        code2: codes[0],
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
            const result = window.main.multithread(codes, codes.map(() => codes[code1]), getTypes(types));
            if (result.includes(-1)) {
                enqueueSnackbar('Error occurred in parsing', { variant: 'error' });
            } else {
                setSimilarities(result);
            }
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

    return (
        <div>
            <ExpansionPanel>
                <ExpansionPanelSummary>sample code</ExpansionPanelSummary>
                <ExpansionPanelDetails className={classes.codeContainer}>
                    <Typography variant='h4'>code 1</Typography>
                    <Divider />
                    <CodeHighlight language='cpp'>{codes[code1]}</CodeHighlight>
                </ExpansionPanelDetails>
            </ExpansionPanel>
            <div className={classes.itemContainer}>
                <TextField
                    select
                    label='code1'
                    value={code1}
                    onChange={setCode}
                >
                    {codes.map((i, j) => <MenuItem key={j} value={j}>{j}</MenuItem>)}
                </TextField>
                <TextField
                    label='types'
                    value={types}
                    onChange={handleInput}
                />
                <Button onClick={run}>Multithreading</Button>
                <Button onClick={sendRequest}>Send Request</Button>
            </div>
            <div className={classes.resultContainer}>
                {similarities.map((i, j) => <div key={j}>
                    <FormControl>
                        <InputLabel htmlFor='similarity'>Similarity</InputLabel>
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
