import { Button, FormControl, Input, InputAdornment, InputLabel, MenuItem, TextField } from '@material-ui/core';
import { useSnackbar } from 'notistack';
import React, { ChangeEvent, useState } from 'react';
import codes from '../../samples/R2.json';
import { useStyles } from '../../styles/style';
import { Expansion } from '../Expansion';

export const R2 = () => {
    const { enqueueSnackbar } = useSnackbar();
    const [similarity, setSimilarity] = useState(0);
    const classes = useStyles();
    const [code1, setCode1] = useState(0);
    const [code2, setCode2] = useState(0);

    const run = () => {
        try {
            setSimilarity(window.main.compareString(codes[code1], codes[code2]) * 100);
        } catch ({ message }) {
            enqueueSnackbar(message, { variant: 'error' });
        }
    };

    const setCode = (num: number) => (event: ChangeEvent<{ name?: string; value: unknown }>) => {
        setSimilarity(0);
        switch (num) {
            case 1:
                setCode1(event.target.value as number);
                break;
            case 2:
                setCode2(event.target.value as number);
                break;
            default:
                break;
        }
    };

    return (
        <div>
            <Expansion code1={codes[code1]} code2={codes[code2]} />
            <div className={classes.itemContainer}>
                <TextField
                    select
                    label='code1'
                    value={code1}
                    onChange={setCode(1)}
                >
                    {codes.map((i, j) => <MenuItem key={j} value={j}>{j}</MenuItem>)}

                </TextField>
                <TextField
                    select
                    label='code2'
                    value={code2}
                    onChange={setCode(2)}
                >
                    {codes.map((i, j) => <MenuItem key={j} value={j}>{j}</MenuItem>)}

                </TextField>
                <Button onClick={run}>String Comparision</Button>
            </div>
            <FormControl className={classes.resultContainer}>
                <InputLabel htmlFor='similarity'>Similarity</InputLabel>
                <Input
                    id='similarity'
                    value={similarity}
                    endAdornment={<InputAdornment position='end'>%</InputAdornment>}
                />
            </FormControl>
        </div>
    );
};
